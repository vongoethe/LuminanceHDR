/**
 * @brief Display Adaptive TMO
 *
 * From:
 * Rafal Mantiuk, Scott Daly, Louis Kerofsky.
 * Display Adaptive Tone Mapping.
 * To appear in: ACM Transactions on Graphics (Proc. of SIGGRAPH'08) 27 (3)
 * http://www.mpi-inf.mpg.de/resources/hdr/datmo/
 *
 * This file is a part of LuminanceHDR package, based on pfstmo.
 * ---------------------------------------------------------------------- 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ---------------------------------------------------------------------- 
 * 
 * @author Rafal Mantiuk, <mantiuk@gmail.com>
 *
 * $Id: display_adaptive_tmo.h,v 1.12 2009/02/23 18:46:36 rafm Exp $
 */

#include "display_function.h"
#include "display_size.h"

#include "TonemappingOperators/pfstmo.h"
#include "Libpfs/pfs.h"

namespace pfs
{
class Progress;
}

class datmoToneCurve
{
  bool own_y_i;
public:
  size_t size;
  const double *x_i;  /* log10 of input luminance factor */
  double *y_i;        /* log10 of output luminance (use inverse display model to get pixel values) */

  datmoToneCurve();
  ~datmoToneCurve();

  void init( size_t n_size, const double *n_x_i, double *n_y_i = NULL );
  void free();
  
};

class datmoConditionalDensity
{
public:
  virtual ~datmoConditionalDensity();
};


/**
 * Computes image statistics required for
 * datmo_compute_tone_curve(). This is the most time-consuming
 * function. If interactive tuning of the TMO parameters is needed,
 * this function can be executed once per image and then
 * datmo_compute_tone_curve() can be executed as many times as needed.
 * 
 * @param width image width in pixels
 * @param height image height in pixels
 * @param L input luminance map (L=0.212656*R + 0.715158*G + 0.072186*B)
 * @param progress_cb callback function for reporting progress or stopping computations.
 * @return pointer to conditional_density or NULL if computation was
 * aborted or an error was encountered. The conditional_density object
 * must be freed by the calling application using the 'delete'
 * statement.
 */
std::auto_ptr<datmoConditionalDensity> datmo_compute_conditional_density( int width, int height, const float *L, pfs::Progress &ph );


/**
 * Computes the best tone-curve for a given conditional_density and
 * TMO parameters. The conditional_density must be computed with
 * datmo_compute_conditional_density() and freed afted execiting this
 * function.
 *
 * @param tc datmoToneCurve where the resulting tone-curve will be stored
 * @param conditional_density image statistics computed with datmo_compute_conditional_density()
 * @param df display function. See DisplayFunction class documentation for more details.
 * @param ds display size. See DisplaySize class documentation for more details.
 * @param enh_factor conrast enhancement factor. See man
 * pfstmo_mantiuk08 page for details
 * @param white_y luminance factor in the input image that should be
 * mapped to the maximum luminance of a display. If the parameter is
 * set to -1, the tone-mapper will not anchor to white (recommended for HDR images).
 * @param progress_cb callback function for reporting progress or stopping computations.
 * @return PFSTMO_OK if tone-mapping was sucessful, PFSTMO_ABORTED if
 * it was stopped from a callback function and PFSTMO_ERROR if an
 * error was encountered.
 */
int datmo_compute_tone_curve( datmoToneCurve *tc, datmoConditionalDensity *cond_dens,
  DisplayFunction *df, DisplaySize *ds, const float enh_factor /*= 1.f*/,
  const float white_y /*= -1*/, pfs::Progress &ph /*= NULL*/ );

/**
 * Deprectaied: use datmo_apply_tone_curve_cc()
 * 
 * Tone-map image using the tone-curve computed with datmo_compute_tone_curve().
 * 
 * @param R_out output red-channel (in pixel values). Can be the same
 * as R_in.
 * @param G_out output green-channel (in pixel values). Can be the same
 * as G_in.
 * @param B_out output blue-channel (in pixel values). Can be the same
 * as B_in.
 * @param width image width in pixels
 * @param height image height in pixels
 * @param R_in red input radiance map.
 * @param G_in green input radiance map.
 * @param B_in blue input radiance map.
 * @param L_in input luminance map (L=0.212656*R + 0.715158*G + 0.072186*B)
 * @param tc tone-curve computed with datmo_compute_tone_curve()
 * @param saturation_factor color saturation factor. See man
 * pfstmo_mantiuk08 page for details
 * @return PFSTMO_OK if tone-mapping was sucessful, PFSTMO_ABORTED if
 * it was stopped from a callback function and PFSTMO_ERROR if an
 * error was encountered.
 */
int datmo_apply_tone_curve( float *R_out, float *G_out, float *B_out, int width, int height,
  const float *R_in, const float *G_in, const float *B_in, const float *L_in, datmoToneCurve *tc,
  DisplayFunction *df, const float saturation_factor = 0.4f );

/**
 * Tone-map image using the tone-curve computed with
 * datmo_compute_tone_curve(). This function corrects color saturation
 * using the method from:
 *
 * Color Correction for Tone Mapping
 * Radosław Mantiuk, Rafał Mantiuk, Anna Tomaszewska, Wolfgang Heidrich.
 * In: Computer Graphics Forum (Proc. of EUROGRAPHICS'09), 28(2), 2009
 * 
 * @param R_out output red-channel (in pixel values). Can be the same
 * as R_in.
 * @param G_out output green-channel (in pixel values). Can be the same
 * as G_in.
 * @param B_out output blue-channel (in pixel values). Can be the same
 * as B_in.
 * @param width image width in pixels
 * @param height image height in pixels
 * @param R_in red input radiance map.
 * @param G_in green input radiance map.
 * @param B_in blue input radiance map.
 * @param L_in input luminance map (L=0.212656*R + 0.715158*G + 0.072186*B)
 * @param tc tone-curve computed with datmo_compute_tone_curve()
 * @param saturation_factor color saturation factor. Set to 1 to preserve colors, >1 to increase color saturation, <1 to reduce color saturation.
 * @return PFSTMO_OK if tone-mapping was sucessful, PFSTMO_ABORTED if
 * it was stopped from a callback function and PFSTMO_ERROR if an
 * error was encountered.
 */
int datmo_apply_tone_curve_cc( float *R_out, float *G_out, float *B_out, int width, int height,
  const float *R_in, const float *G_in, const float *B_in, const float *L_in, datmoToneCurve *tc,
  DisplayFunction *df, const float saturation_factor );

#define DATMO_TF_TAPSIZE 26     /* Number of samples required for the temporal filter */
