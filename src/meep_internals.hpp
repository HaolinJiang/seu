/* Copyright (C) 2005-2009 Massachusetts Institute of Technology
%
%  This program is free software; you can redistribute it and/or modify
%  it under the terms of the GNU General Public License as published by
%  the Free Software Foundation; either version 2, or (at your option)
%  any later version.
%
%  This program is distributed in the hope that it will be useful,
%  but WITHOUT ANY WARRANTY; without even the implied warranty of
%  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%  GNU General Public License for more details.
%
%  You should have received a copy of the GNU General Public License
%  along with this program; if not, write to the Free Software Foundation,
%  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "meep.hpp"

namespace meep {

#define DOCMP for (int cmp=0;cmp<2-is_real;cmp++)
#define DOCMP2 for (int cmp=0;cmp<2;cmp++)

inline double max(double a, double b) { return (a > b) ? a : b; }
inline double min(double a, double b) { return (a < b) ? a : b; }
inline int max(int a, int b) { return (a > b) ? a : b; }
inline int min(int a, int b) { return (a < b) ? a : b; }
static inline int abs(int a) { return a < 0 ? -a : a; }
static inline double abs(double a) { return fabs(a); }

// note that C99 has a round() function, but I don't want to rely on it
static inline int my_round(double x) {
  return int(floor(fabs(x) + 0.5) * (x < 0 ? -1 : 1));
}

inline int small_r_metal(int m) {
  return m-1;
}

inline int rmin_bulk(int m) {
  int r = 1 + small_r_metal(m);
  if (r < 1) r = 1;
  return r;
}

class polarizability {
 public:
  volume v;
  polarizability(const structure_chunk *, material_function &sig,
                 field_type ft, double om, double ga, double sigscale,
                 bool mine = true);
  polarizability(const polarizability *);
  ~polarizability();
  field_type ft;
  double gamma, omeganot;
  realnum *s[NUM_FIELD_COMPONENTS];
  bool is_mine() { return is_it_mine; };
  bool is_it_mine;
  polarizability *next;

  polarizability_identifier get_identifier() const;
};

class polarization {
 public:
  polarization(const polarizability *the_pb, int is_real, bool store_enrgy=0);
  ~polarization();
  realnum *(P[NUM_FIELD_COMPONENTS][2]), *(energy[NUM_FIELD_COMPONENTS]),
    *(s[NUM_FIELD_COMPONENTS]);
  int is_real;
  bool store_energy;
  const polarizability *pb;
  polarization *next;

  complex<double> analytic_chi1(component,double freq, const vec &) const;
  double local_energy(const ivec &);
  // for total energy, use fields::thermo_energy_in_box
  static void set_up_polarizations(polarization *pols[NUM_FIELD_TYPES], const structure_chunk *s, int is_real, bool store_enrgy = 0);
  void use_real_fields();
  void zero_fields();
};

class src_vol {
 public:
  src_vol(component cc, src_time *st, int n, int *ind, complex<double> *amps);
  src_vol(const src_vol &sv);
  ~src_vol() { delete next; delete[] index; delete[] A;}

  src_time *t;
  int *index; // list of locations of sources in grid (indices)
  int npts; // number of points in list
  component c; // field component the source applies to
  complex<double> *A; // list of amplitudes

  complex<double> dipole(int j) { return A[j] * t->dipole(); }
  complex<double> current(int j) { return A[j] * t->current(); }
  void update(double time, double dt) { t->update(time, dt); }

  bool operator==(const src_vol &sv) const {
    return sv.index[0]==index[0] && sv.index[sv.npts-1]==index[npts-1] && sv.c==c && sv.t==t;
  }

  src_vol *add_to(src_vol *others);
  src_vol *next;
};

const int num_bandpts = 32;

class bandsdata {
 public:
  bandsdata();
  ~bandsdata();

  complex<double> *f[num_bandpts][NUM_FIELD_COMPONENTS];
  // The following is the polarization at just one point, with Pz and Pp
  // added together (a crude compromize for speed, while still observing the
  // phonon bands).
  complex<double> *P;
  int tstart, tend, index[num_bandpts], maxbands, scale_factor;
  fields_chunk *chunk[num_bandpts];
  double dt, fmin, fmax, qmin, fpmin;
  int ntime;
  int verbosity;

  int get_freqs(complex<double> *data, int n,
                complex<double> *amps, double *freqs, double *decays);
  int look_for_more_bands(complex<double> *simple_data,
                          double *reff, double *refd,
                          complex<double> *refa,
                          complex<double> *refdata,
                          int numref);
};

symmetry r_to_minus_r_symmetry(int m);

#define MIN_OUTPUT_TIME 4.0 // output no more often than this many seconds


// functions in step_generic.cpp:

void step_curl(realnum *f, component c, const realnum *g1, const realnum *g2,
	       int s1, int s2, // strides for g1/g2 shift
	       const volume &v, double dtdx,
	       direction dsig, const double *sig, const double *siginv,
	       double dt, const realnum *cnd, const realnum *cndinv);

void step_update_EDHB(realnum *f, component fc, const volume &v,
		      const realnum *g, const realnum *g1, const realnum *g2,
		      const realnum *gb, const realnum *g1b, const realnum *g2b,
		      const realnum *u, const realnum *u1, const realnum *u2,
		      int s, int s1, int s2,
		      const realnum *chi2, const realnum *chi3,
		      direction dsig,const double *sig,const double *siginv,
		      direction dsigg, const double *sigg,
		      direction dsig1, const double *sig1,
		      direction dsig1inv, const double *sig1inv,
		      direction dsig2, const double *sig2,
		      direction dsig2inv, const double *sig2inv,
		      int sigsize_dsig,int sigsize_dsigg,int sigsize_dsig1);


// functions in step_generic_stride1.cpp, generated from step_generic.cpp:

void step_curl_stride1(realnum *f, component c, const realnum *g1, const realnum *g2,
	       int s1, int s2, // strides for g1/g2 shift
	       const volume &v, double dtdx,
	       direction dsig, const double *sig, const double *siginv,
	       double dt, const realnum *cnd, const realnum *cndinv);

void step_update_EDHB_stride1(realnum *f, component fc, const volume &v,
		      const realnum *g, const realnum *g1, const realnum *g2,
		      const realnum *gb, const realnum *g1b, const realnum *g2b,
		      const realnum *u, const realnum *u1, const realnum *u2,
		      int s, int s1, int s2,
		      const realnum *chi2, const realnum *chi3,
		      direction dsig,const double *sig,const double *siginv,
		      direction dsigg, const double *sigg,
		      direction dsig1, const double *sig1,
		      direction dsig1inv, const double *sig1inv,
		      direction dsig2, const double *sig2,
		      direction dsig2inv, const double *sig2inv,
		      int sigsize_dsig,int sigsize_dsigg,int sigsize_dsig1);

/* macro wrappers around time-stepping functions: for performance reasons,
   if the inner loop is stride-1 then we use the stride-1 versions,
   which allow gcc (and possibly other compilers) to do additional
   optimizations, especially loop vectorization */

#define STEP_CURL(f, c, g1, g2, s1, s2, v, dtdx, dsig, sig, siginv, dt, cnd, cndinv) do { \
  if (LOOPS_ARE_STRIDE1(v))						\
    step_curl_stride1(f, c, g1, g2, s1, s2, v, dtdx, dsig, sig, siginv, dt, cnd, cndinv); \
  else									\
    step_curl(f, c, g1, g2, s1, s2, v, dtdx, dsig, sig, siginv, dt, cnd, cndinv); \
} while (0)

#define STEP_UPDATE_EDHB(f, fc, v, g, g1, g2, gb, g1b, g2b, u, u1, u2, s, s1, s2, chi2, chi3, dsig, sig, siginv, dsigg, sigg, dsig1, sig1, dsig1inv, sig1inv, dsig2, sig2, dsig2inv, sig2inv, sigsize_dsig, sigsize_dsigg, sigsize_dsig1) do { \
  if (LOOPS_ARE_STRIDE1(v))						\
    step_update_EDHB_stride1(f, fc, v, g, g1, g2, gb, g1b, g2b, u, u1, u2, s, s1, s2, chi2, chi3, dsig, sig, siginv, dsigg, sigg, dsig1, sig1, dsig1inv, sig1inv, dsig2, sig2, dsig2inv, sig2inv, sigsize_dsig, sigsize_dsigg, sigsize_dsig1); \
  else									\
    step_update_EDHB(f, fc, v, g, g1, g2, gb, g1b, g2b, u, u1, u2, s, s1, s2, chi2, chi3, dsig, sig, siginv, dsigg, sigg, dsig1, sig1, dsig1inv, sig1inv, dsig2, sig2, dsig2inv, sig2inv, sigsize_dsig, sigsize_dsigg, sigsize_dsig1); \
} while (0)

} // namespace meep
