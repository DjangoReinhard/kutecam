/* 
 * **************************************************************************
 * 
 *  file:       HelixCurveAdaptor.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements
 *  created:    01.08.2015 by Roman Lygin
 *  copyright:  (c) 2015 Roman Lygin -  all rights reserved
 *
 *  taken from https://opencascade.blogspot.com/2015/08/arbitrary-law-based-curve-and-surface.html
 *
 * **************************************************************************
 */
#ifndef HELIXCURVEADAPTOR_H
#define HELIXCURVEADAPTOR_H
#include <Standard_Version.hxx>
#include <Adaptor3d_Curve.hxx>
#include <memory>
class Geom_HelixData;

/*! A few methods in OCC 6.9.0 have been made const.*/
#if OCC_VERSION_HEX < 0x060900
#define __CADEX_ADAPTOR3D_CURVE_CONST
#else
#define __CADEX_ADAPTOR3D_CURVE_CONST const
#endif


/*
 * \class ACISAlgo_HelixCurveAdaptor
 * \brief Defines an adaptor to represent a helix curve.
 *
 * Helix data is defined by ACISGeom_HelixData.
 * Evaluation is performed in the Evaluator subclass
 * which can either represent a helix lying on a
 * cylinder (if taper is 0) or on a cone (if taper is not 0).
 *
 * Helix can have distinct radii along X and Y axes,
 * i.e. to have an elliptical section.
 */
class HelixCurveAdaptor : public Adaptor3d_Curve
{
public:
  class Evaluator;

  //! Constructor.
  HelixCurveAdaptor(const Geom_HelixData& theData);

  //! Constructor
  HelixCurveAdaptor(const std::shared_ptr<Evaluator>& theEvaluator
                           , double theMin
                           , double theMax);

  virtual double FirstParameter() const override;
  virtual double LastParameter()  const override;

  virtual GeomAbs_Shape Continuity() const override;
  virtual Standard_Integer NbIntervals(const GeomAbs_Shape S) __CADEX_ADAPTOR3D_CURVE_CONST override;
  virtual void Intervals(TColStd_Array1OfReal& T, const GeomAbs_Shape S) __CADEX_ADAPTOR3D_CURVE_CONST override;
  virtual Handle(Adaptor3d_Curve) Trim(const double First
                                     , const double Last
                                     , const double Tol) const override;

  virtual Standard_Boolean IsClosed() const override;
  virtual Standard_Boolean IsPeriodic() const override;

  virtual gp_Pnt Value(const double U) const override;
  virtual void D0(const double U, gp_Pnt& P) const override;
  virtual void D1(const double U, gp_Pnt& P, gp_Vec& V) const override;
  virtual void D2(const double U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2) const override;
  virtual void D3(const double U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2, gp_Vec& V3) const override;
  virtual gp_Vec DN(const double U, const Standard_Integer N) const override;
  virtual double Resolution(const double R3d) const override;
  virtual GeomAbs_CurveType GetType() const override;

protected:
  std::shared_ptr<Evaluator> myEvaluator;
  double                     myMin;
  double                     myMax;
  };

#include "HHelixCurveAdaptor.h"
#endif
