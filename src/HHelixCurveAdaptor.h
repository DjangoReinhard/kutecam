/* 
 * **************************************************************************
 * 
 *  file:       HHelixCurveAdaptor.h
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
#ifndef HHELIXCURVEADAPTOR_H
#define HHELIXCURVEADAPTOR_H
#include "Geom_HelixData.h"
#include "HelixCurveAdaptor.h"
#include <Standard_Type.hxx>
#include <Standard_DefineHandle.hxx>
#include <Adaptor3d_Curve.hxx>
#include <memory>
#include <QDebug>

DEFINE_STANDARD_HANDLE(ACISAlgo_HHelixCurveAdaptor, Adaptor3d_Curve)


class HHelixCurveAdaptor : public Adaptor3d_Curve
{
  DEFINE_STANDARD_RTTIEXT(HHelixCurveAdaptor, Adaptor3d_Curve)

public:
  //! Constructor.
  HHelixCurveAdaptor(const Geom_HelixData& theData);

  //! Constructor.
  HHelixCurveAdaptor(const std::shared_ptr<HelixCurveAdaptor::Evaluator>& theEvaluator
                            , Standard_Real theMin
                            , Standard_Real theMax);

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
  HelixCurveAdaptor myAdaptor;
  };
#endif
