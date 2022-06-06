/* 
 * **************************************************************************
 * 
 *  file:       HelixCurveAdaptor_CylinderEvaluator.h
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
#ifndef HELIXCURVEADAPTOR_CYLINDEREVALUATOR_H
#define HELIXCURVEADAPTOR_CYLINDEREVALUATOR_H
#include "HelixCurveAdaptor.h"
#include "HelixCurveAdaptor_p.h"
#include "Geom_HelixData.h"
#include <gp_Pnt.hxx>


/*!
 * \class ACISAlgo_HelixCurveAdaptor_CylinderEvaluator
 * \brief Evaluates a helix lying on a cylinder.
 */
class HelixCurveAdaptor_CylinderEvaluator : public HelixCurveAdaptor::Evaluator
{
public:
  HelixCurveAdaptor_CylinderEvaluator(const Geom_HelixData& theData);

  virtual void   D0(double U, gp_Pnt& P) const override;
  virtual void   D1(double U, gp_Pnt& P, gp_Vec& V) const override;
  virtual void   D2(double U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2) const override;
  virtual void   D3(double U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2, gp_Vec& V3) const override;
  virtual gp_Vec DN(double U, int N) const override;
  };

#endif
