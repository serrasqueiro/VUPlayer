/*
 *  ReplayGainAnalysis - analyzes input samples and give the recommended dB change
 *  Copyright (C) 2001 David Robinson and Glen Sawyer
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  concept and filter values by David Robinson (David@Robinson.org)
 *    -- blame him if you think the idea is flawed
 *  original coding by Glen Sawyer (glensawyer@hotmail.com)
 *    -- blame him if you think this runs too slowly, or the coding is otherwise flawed
 *
 *  lots of code improvements by Frank Klemm ( http://www.uni-jena.de/~pfk/mpp/ )
 *    -- credit him for all the _good_ programming ;)
 *
 *  minor cosmetic tweaks to integrate with FLAC by Josh Coalson
 *
 *	thread safe C++ version by James Chapman
 *
 *  For an explanation of the concepts and the basic algorithms involved, go to:
 *    http://www.replaygain.org/
 */

/*
 *  Here's the deal. Call
 *
 *    InitGainAnalysis ( long samplefreq );
 *
 *  to initialize everything. Call
 *
 *    AnalyzeSamples ( const flac_float_t*  left_samples,
 *                     const flac_float_t*  right_samples,
 *                     size_t          num_samples,
 *                     int             num_channels );
 *
 *  as many times as you want, with as many or as few samples as you want.
 *  If mono, pass the sample buffer in through left_samples, leave
 *  right_samples NULL, and make sure num_channels = 1.
 *
 *    GetTitleGain()
 *
 *  will return the recommended dB level change for all samples analyzed
 *  SINCE THE LAST TIME you called GetTitleGain() OR InitGainAnalysis().
 *
 *    GetAlbumGain()
 *
 *  will return the recommended dB level change for all samples analyzed
 *  since InitGainAnalysis() was called and finalized with GetTitleGain().
 *
 *  Pseudo-code to process an album:
 *
 *    flac_float_t       l_samples [4096];
 *    flac_float_t       r_samples [4096];
 *    size_t        num_samples;
 *    unsigned int  num_songs;
 *    unsigned int  i;
 *
 *    InitGainAnalysis ( 44100 );
 *    for ( i = 1; i <= num_songs; i++ ) {
 *        while ( ( num_samples = getSongSamples ( song[i], left_samples, right_samples ) ) > 0 )
 *            AnalyzeSamples ( left_samples, right_samples, num_samples, 2 );
 *        fprintf ("Recommended dB change for song %2d: %+6.2f dB\n", i, GetTitleGain() );
 *    }
 *    fprintf ("Recommended dB change for whole album: %+6.2f dB\n", GetAlbumGain() );
 */

/*
 *  So here's the main source of potential code confusion:
 *
 *  The filters applied to the incoming samples are IIR filters,
 *  meaning they rely on up to <filter order> number of previous samples
 *  AND up to <filter order> number of previous filtered samples.
 *
 *  I set up the AnalyzeSamples routine to minimize memory usage and interface
 *  complexity. The speed isn't compromised too much (I don't think), but the
 *  internal complexity is higher than it should be for such a relatively
 *  simple routine.
 *
 *  Optimization/clarity suggestions are welcome.
 */

#include "replaygain_analysis.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

float ReplayGainReferenceLoudness = 89.0; /* in dB SPL */

#define RMS_PERCENTILE      0.95        /* percentile which is louder than the proposed level */
#define RMS_WINDOW_TIME    50           /* Time slice size [ms] */
#define STEPS_per_dB      100.          /* Table entries per dB */
#define MAX_dB            120.          /* Table entries for 0...MAX_dB (normal max. values are 70...80 dB) */

#define MAX_ORDER               (BUTTER_ORDER > YULE_ORDER ? BUTTER_ORDER : YULE_ORDER)
#define PINK_REF                64.82 /* 298640883795 */                          /* calibration value */

static const int ReplayGainFilterCount = 13;
static const ReplayGainFilter ReplayGainFilters[ ReplayGainFilterCount ] = {

    {
        48000, 0, /* ORIGINAL */
        { 0.03857599435200,  -0.02160367184185,  -0.00123395316851,  -0.00009291677959,  -0.01655260341619,   0.02161526843274,  -0.02074045215285,   0.00594298065125,   0.00306428023191,   0.00012025322027,   0.00288463683916 },
        { 1.00000000000000,  -3.84664617118067,   7.81501653005538, -11.34170355132042,  13.05504219327545, -12.28759895145294,   9.48293806319790, -5.87257861775999,   2.75465861874613,   -0.86984376593551,   0.13919314567432 },
        { 0.98621192462708,  -1.97242384925416,   0.98621192462708 },
        { 1.00000000000000,  -1.97223372919527,   0.97261396931306 },
    },

    {
        44100, 0, /* ORIGINAL */
        { 0.05418656406430,  -0.02911007808948,  -0.00848709379851,  -0.00851165645469,  -0.00834990904936,   0.02245293253339,  -0.02596338512915,   0.01624864962975,  -0.00240879051584,   0.00674613682247,  -0.00187763777362 },
        { 1.00000000000000,  -3.47845948550071,   6.36317777566148,  -8.54751527471874,   9.47693607801280,  -8.81498681370155,   6.85401540936998,  -4.39470996079559,   2.19611684890774,  -0.75104302451432,   0.13149317958808 },
        { 0.98500175787242,  -1.97000351574484,   0.98500175787242 },
        { 1.00000000000000,  -1.96977855582618,   0.97022847566350 },
    },

    {
        37800, 0,
        { 0.10296717174470,  -0.04877975583256,  -0.02878009075237,  -0.03519509188311,   0.02888717172493,  -0.00609872684844,   0.00209851217112,   0.00911704668543,   0.01154404718589,  -0.00630293688700,   0.00107527155228 },
        { 1.00000000000000,  -2.64848054923531,   3.58406058405771,  -3.83794914179161,   3.90142345804575,  -3.50179818637243,   2.67085284083076,  -1.82581142372418,   1.09530368139801,  -0.47689017820395,   0.11171431535905 },
        { 0.98252400815195,  -1.96504801630391,   0.98252400815195 },
        { 1.00000000000000,  -1.96474258269041,   0.96535344991740 },
    },

    {
        36000, 0,
        { 0.11572297028613,  -0.04120916051252,  -0.04977731768022,  -0.01047308680426,   0.00750863219157,   0.00055507694408,   0.00140344192886,   0.01286095246036,   0.00998223033885,  -0.00725013810661,   0.00326503346879 },
        { 1.00000000000000,  -2.43606802820871,   3.01907406973844,  -2.90372016038192,   2.67947188094303,  -2.17606479220391,   1.44912956803015,  -0.87785765549050,   0.53592202672557,  -0.26469344817509,   0.07495878059717 },
        { 0.98165826840326,  -1.96331653680652,   0.98165826840326 },
        { 1.00000000000000,  -1.96298008938934,   0.96365298422371 },
    },

    {
        32000, 0, /* ORIGINAL */
        { 0.15457299681924,  -0.09331049056315,  -0.06247880153653,   0.02163541888798,  -0.05588393329856,   0.04781476674921,   0.00222312597743,   0.03174092540049,  -0.01390589421898,   0.00651420667831,  -0.00881362733839 },
        { 1.00000000000000,  -2.37898834973084,   2.84868151156327,  -2.64577170229825,   2.23697657451713,  -1.67148153367602,   1.00595954808547,  -0.45953458054983,   0.16378164858596,  -0.05032077717131,   0.02347897407020 },
        { 0.97938932735214,  -1.95877865470428,   0.97938932735214 },
        { 1.00000000000000,  -1.95835380975398,   0.95920349965459 },
    },

    {
        28000, 0,
        { 0.23882392323383,  -0.22007791534089,  -0.06014581950332,   0.05004458058021,  -0.03293111254977,   0.02348678189717,   0.04290549799671,  -0.00938141862174,   0.00015095146303,  -0.00712601540885,  -0.00626520210162 },
        { 1.00000000000000,  -2.06894080899139,   1.76944699577212,  -0.81404732584187,   0.25418286850232,  -0.30340791669762,   0.35616884070937,  -0.14967310591258,  -0.07024154183279,   0.11078404345174,  -0.03551838002425 },
        { 0.97647981663949,  -1.95295963327897,   0.97647981663949 },
        { 1.00000000000000,  -1.95240635772520,   0.95351290883275 },

    },

    {
        24000, 0, /* ORIGINAL */
        { 0.30296907319327,  -0.22613988682123,  -0.08587323730772,   0.03282930172664,  -0.00915702933434,  -0.02364141202522,  -0.00584456039913,   0.06276101321749,  -0.00000828086748,   0.00205861885564,  -0.02950134983287 },
        { 1.00000000000000,  -1.61273165137247,   1.07977492259970,  -0.25656257754070,  -0.16276719120440,  -0.22638893773906,   0.39120800788284,  -0.22138138954925,   0.04500235387352,   0.02005851806501,   0.00302439095741 },
        { 0.97531843204928,  -1.95063686409857,   0.97531843204928 },
        { 1.00000000000000,  -1.95002759149878,   0.95124613669835 },
    },

    {
        22050, 0, /* ORIGINAL */
        { 0.33642304856132,  -0.25572241425570,  -0.11828570177555,   0.11921148675203,  -0.07834489609479,  -0.00469977914380,  -0.00589500224440,   0.05724228140351,   0.00832043980773,  -0.01635381384540,  -0.01760176568150 },
        { 1.00000000000000,  -1.49858979367799,   0.87350271418188,   0.12205022308084,  -0.80774944671438,   0.47854794562326,  -0.12453458140019,  -0.04067510197014,   0.08333755284107,  -0.04237348025746,   0.02977207319925 },
        { 0.97316523498161,  -1.94633046996323,   0.97316523498161 },
        { 1.00000000000000,  -1.94561023566527,   0.94705070426118 },
    },

    {
        18900, 0,
        { 0.38412657295385,  -0.44533729608120,   0.20426638066221,  -0.28031676047946,   0.31484202614802,  -0.26078311203207,   0.12925201224848,  -0.01141164696062,   0.03036522115769,  -0.03776339305406,   0.00692036603586 },
        { 1.00000000000000,  -1.74403915585708,   1.96686095832499,  -2.10081452941881,   1.90753918182846,  -1.83814263754422,   1.36971352214969,  -0.77883609116398,   0.39266422457649,  -0.12529383592986,   0.05424760697665 },
        { 0.96535326815829,  -1.93070653631658,   0.96535326815829 },
        { 1.00000000000000,  -1.92950577983524,   0.93190729279793 },
    },

    {
        16000, 0, /* ORIGINAL */
        { 0.44915256608450,  -0.14351757464547,  -0.22784394429749,  -0.01419140100551,   0.04078262797139,  -0.12398163381748,   0.04097565135648,   0.10478503600251,  -0.01863887810927,  -0.03193428438915,   0.00541907748707 },
        { 1.00000000000000,  -0.62820619233671,   0.29661783706366,  -0.37256372942400,   0.00213767857124,  -0.42029820170918,   0.22199650564824,   0.00613424350682,   0.06747620744683,   0.05784820375801,   0.03222754072173 },
        { 0.96454515552826,  -1.92909031105652,   0.96454515552826 },
        { 1.00000000000000,  -1.92783286977036,   0.93034775234268 },
    },

    {
        12000, 0, /* ORIGINAL */
        { 0.56619470757641,  -0.75464456939302,   0.16242137742230,   0.16744243493672,  -0.18901604199609,   0.30931782841830,  -0.27562961986224,   0.00647310677246,   0.08647503780351,  -0.03788984554840,  -0.00588215443421 },
        { 1.00000000000000,  -1.04800335126349,   0.29156311971249,  -0.26806001042947,   0.00819999645858,   0.45054734505008,  -0.33032403314006,   0.06739368333110,  -0.04784254229033,   0.01639907836189,   0.01807364323573 },
        { 0.96009142950541,  -1.92018285901082,   0.96009142950541 },
        { 1.00000000000000,  -1.91858953033784,   0.92177618768381 },
    },

    {
        11025, 0, /* ORIGINAL */
        { 0.58100494960553,  -0.53174909058578,  -0.14289799034253,   0.17520704835522,   0.02377945217615,   0.15558449135573,  -0.25344790059353,   0.01628462406333,   0.06920467763959,  -0.03721611395801,  -0.00749618797172 },
        { 1.00000000000000,  -0.51035327095184,  -0.31863563325245,  -0.20256413484477,   0.14728154134330,   0.38952639978999,  -0.23313271880868,  -0.05246019024463,  -0.02505961724053,   0.02442357316099,   0.01818801111503 },
        { 0.95856916599601,  -1.91713833199203,   0.95856916599601 },
        { 1.00000000000000,  -1.91542108074780,   0.91885558323625 },
    },

    {
        8000, 0, /* ORIGINAL */
        { 0.53648789255105,  -0.42163034350696,  -0.00275953611929,   0.04267842219415,  -0.10214864179676,   0.14590772289388,  -0.02459864859345,  -0.11202315195388,  -0.04060034127000,   0.04788665548180,  -0.02217936801134 },
        { 1.00000000000000,  -0.25049871956020,  -0.43193942311114,  -0.03424681017675,  -0.04678328784242,   0.26408300200955,   0.15113130533216,  -0.17556493366449,  -0.18823009262115,   0.05477720428674,   0.04704409688120 },
        { 0.94597685600279,  -1.89195371200558,   0.94597685600279 },
        { 1.00000000000000,  -1.88903307939452,   0.89487434461664 },
    },

};

replaygain_analysis::replaygain_analysis() :
	m_gainfilter( {} ),
	m_linprebuf( MAX_ORDER * 2, 0 ),
	m_rinprebuf( MAX_ORDER * 2, 0 ),
	m_lstepbuf(),
	m_rstepbuf(),
	m_loutbuf(),
	m_routbuf(),
	m_A( 120 * 100, 0 ),
	m_B( 120 * 100, 0 ),
	m_linpre( nullptr ),
	m_lstep( nullptr ),
	m_lout( nullptr ),
	m_rinpre( nullptr ),
	m_rstep( nullptr ),
	m_rout( nullptr ),
	m_sampleWindow( 0 ),
	m_totsamp( 0 )
{
}

replaygain_analysis::~replaygain_analysis()
{
}

void replaygain_analysis::filter( const float* input, float* output, size_t nSamples, const float* a, const float* b, const size_t order, const unsigned int downsample ) const
{
    double  y;
    size_t  i;
    size_t  k;

    const float* input_head = input;
    const float* input_tail;

    float* output_head = output;
    float* output_tail;

    for ( i = 0; i < nSamples; i++, input_head += downsample, ++output_head ) {

        input_tail = input_head;
        output_tail = output_head;

        y = *input_head * b[0];

        for ( k = 1; k <= order; k++ ) {
            input_tail -= downsample;
            --output_tail;
            y += *input_tail * b[k] - *output_tail * a[k];
        }

        output[i] = static_cast<float>( y );
    }
}

bool replaygain_analysis::CreateGainFilter( long samplefreq, ReplayGainFilter& gainfilter ) const
{
	bool success = false;

    unsigned int i;
    long maxrate = 0;
    unsigned int downsample = 1;

    while ( !success ) {
        for ( i = 0; i < ReplayGainFilterCount; ++i ) {
            if (maxrate < ReplayGainFilters[i].rate) {
                maxrate = ReplayGainFilters[i].rate;
						}

            if ( ReplayGainFilters[i].rate == samplefreq ) {
                gainfilter = ReplayGainFilters[i];
                gainfilter.downsample = downsample;
                success = true;
								break;
            }
        }

        if (samplefreq < maxrate)
            break;

        while (samplefreq > maxrate) {
            downsample *= 2;
            samplefreq /= 2;
        }
    }

    return success;
}

bool replaygain_analysis::ResetSampleFrequency( const long samplefreq )
{
	bool success = false;
	if ( CreateGainFilter( samplefreq, m_gainfilter ) ) {
		m_sampleWindow = ( m_gainfilter.rate * RMS_WINDOW_TIME + 1000-1 ) / 1000;
		m_lstepbuf.resize( m_sampleWindow + MAX_ORDER, 0 );
		m_rstepbuf.resize( m_sampleWindow + MAX_ORDER, 0 );
		m_loutbuf.resize( m_sampleWindow + MAX_ORDER, 0 );
		m_routbuf.resize( m_sampleWindow + MAX_ORDER, 0 );

    /* zero out initial values */
    for ( int i = 0; i < MAX_ORDER; i++ ) {
        m_linprebuf[i] = m_lstepbuf[i] = m_loutbuf[i] = m_rinprebuf[i] = m_rstepbuf[i] = m_routbuf[i] = 0;
		}
		m_lsum = 0;
		m_rsum = 0;
		m_totsamp = 0;
		std::fill( m_A.begin(), m_A.end(), 0 );
		std::fill( m_B.begin(), m_B.end(), 0 );

		success = true;
	}
	return success;
}

bool replaygain_analysis::ValidGainFrequency ( const long samplefreq ) const 
{
	ReplayGainFilter gainfilter = {};
	const bool valid = CreateGainFilter( samplefreq, gainfilter );
	return valid;
}

bool replaygain_analysis::InitGainAnalysis( const long samplefreq )
{
	bool success = false;
	if ( ResetSampleFrequency( samplefreq ) ) {
    m_linpre       = &(m_linprebuf[MAX_ORDER]);
    m_rinpre       = &(m_rinprebuf[MAX_ORDER]);
    m_lstep        = &(m_lstepbuf[MAX_ORDER]);
    m_rstep        = &(m_rstepbuf[MAX_ORDER]);
    m_lout         = &(m_loutbuf[MAX_ORDER]);
    m_rout         = &(m_routbuf[MAX_ORDER]);

		success = true;
	}
	return success;
}

bool replaygain_analysis::AnalyzeSamples( const float* left_samples, const float* right_samples, size_t num_samples, int num_channels )
{
    unsigned int       downsample = m_gainfilter.downsample;
    const float*  curleft;
    const float*  curright;
    long            prebufsamples;
    long            batchsamples;
    long            cursamples;
    long            cursamplepos;
    int             i;

    num_samples /= downsample;

    if ( num_samples == 0 )
        return true;

    cursamplepos = 0;
    batchsamples = num_samples;

    switch ( num_channels) {
    case  1: right_samples = left_samples;
    case  2: break;
    default: return false;
    }

    prebufsamples = MAX_ORDER;
    if ((size_t) prebufsamples > num_samples)
        prebufsamples = num_samples;

    for ( i = 0; i < prebufsamples; ++i ) {
        m_linprebuf[i+MAX_ORDER] = left_samples [i * downsample];
        m_rinprebuf[i+MAX_ORDER] = right_samples[i * downsample];
    }

    while ( batchsamples > 0 ) {
        cursamples = batchsamples > (long)(m_sampleWindow-m_totsamp)  ?  (long)(m_sampleWindow - m_totsamp)  :  batchsamples;
        if ( cursamplepos < MAX_ORDER ) {
            downsample = 1;
            curleft  = m_linpre+cursamplepos;
            curright = m_rinpre+cursamplepos;
            if (cursamples > MAX_ORDER - cursamplepos )
                cursamples = MAX_ORDER - cursamplepos;
        }
        else {
            downsample = m_gainfilter.downsample;
            curleft  = left_samples  + cursamplepos * downsample;
            curright = right_samples + cursamplepos * downsample;
        }

        filter ( curleft , m_lstep + m_totsamp, cursamples, m_gainfilter.AYule, m_gainfilter.BYule, YULE_ORDER, downsample );
        filter ( curright, m_rstep + m_totsamp, cursamples, m_gainfilter.AYule, m_gainfilter.BYule, YULE_ORDER, downsample );

        filter ( m_lstep + m_totsamp, m_lout + m_totsamp, cursamples, m_gainfilter.AButter, m_gainfilter.BButter, BUTTER_ORDER, 1 );
        filter ( m_rstep + m_totsamp, m_rout + m_totsamp, cursamples, m_gainfilter.AButter, m_gainfilter.BButter, BUTTER_ORDER, 1 );

        for ( i = 0; i < cursamples; i++ ) {             /* Get the squared values */
            m_lsum += m_lout [m_totsamp+i] * m_lout [m_totsamp+i];
            m_rsum += m_rout [m_totsamp+i] * m_rout [m_totsamp+i];
        }

        batchsamples -= cursamples;
        cursamplepos += cursamples;
        m_totsamp      += cursamples;
        if ( m_totsamp == m_sampleWindow ) {  /* Get the Root Mean Square (RMS) for this set of samples */
            double  val  = STEPS_per_dB * 10. * log10 ( (m_lsum+m_rsum) / m_totsamp * 0.5 + 1.e-37 );
            int     ival = (int) val;
            if ( ival <                     0 ) ival = 0;
            if ( ival >= (int)(m_A.size()) ) ival = (int)(m_A.size()) - 1;
            m_A[ival]++;
            m_lsum = m_rsum = 0.;
            memmove ( &m_loutbuf[0] , &m_loutbuf[m_totsamp], MAX_ORDER * sizeof(float) );
            memmove ( &m_routbuf[0] , &m_routbuf[m_totsamp], MAX_ORDER * sizeof(float) );
            memmove ( &m_lstepbuf[0], &m_lstepbuf[m_totsamp], MAX_ORDER * sizeof(float) );
            memmove ( &m_rstepbuf[0], &m_rstepbuf[m_totsamp], MAX_ORDER * sizeof(float) );
            m_totsamp = 0;
        }
        if ( m_totsamp > m_sampleWindow )   /* somehow I really screwed up: Error in programming! Contact author about totsamp > sampleWindow */
            return false;
    }

    if ( num_samples < MAX_ORDER ) {
        memmove ( &m_linprebuf[0],                           &m_linprebuf[num_samples], (MAX_ORDER-num_samples) * sizeof(float) );
        memmove ( &m_rinprebuf[0],                           &m_rinprebuf[num_samples], (MAX_ORDER-num_samples) * sizeof(float) );
        memcpy  ( &m_linprebuf[MAX_ORDER - num_samples], left_samples,          num_samples             * sizeof(float) );
        memcpy  ( &m_rinprebuf[MAX_ORDER - num_samples], right_samples,         num_samples             * sizeof(float) );
    }
    else {
        downsample = m_gainfilter.downsample;

        left_samples  += (num_samples - MAX_ORDER) * downsample;
        right_samples += (num_samples - MAX_ORDER) * downsample;

        for ( i = 0; i < MAX_ORDER; ++i ) {
            m_linprebuf[i] = left_samples [i * downsample];
            m_rinprebuf[i] = right_samples[i * downsample];
        }
    }

    return true;
}

float replaygain_analysis::analyzeResult( const std::vector<unsigned int>& Array ) const
{

    uint32_t  elems;
    int32_t   upper;
    size_t    i;

    elems = 0;
    for ( i = 0; i < Array.size(); i++ )
        elems += Array[i];
    if ( elems == 0 )
        return GAIN_NOT_ENOUGH_SAMPLES;

    upper = (int32_t) (elems / 20 + ((elems % 20) ? 1 : 0));
    for ( i = Array.size(); i-- > 0; ) {
        if ( (upper -= Array[i]) <= 0 )
            break;
    }

    return (float) ((float)PINK_REF - (float)i / (float)STEPS_per_dB);
}

float replaygain_analysis::GetTitleGain()
{
    float  retval;
    unsigned int    i;

    retval = analyzeResult ( m_A );

    for ( i = 0; i < m_A.size(); i++ ) {
        m_B[i] += m_A[i];
        m_A[i]  = 0;
    }

    for ( i = 0; i < MAX_ORDER; i++ ) {
        m_linprebuf[i] = m_lstepbuf[i] = m_loutbuf[i] = m_rinprebuf[i] = m_rstepbuf[i] = m_routbuf[i] = 0;
		}

    m_totsamp = 0;
    m_lsum = 0;
		m_rsum = 0;
    return retval;
}

float replaygain_analysis::GetAlbumGain()
{
    return analyzeResult( m_B );
}
