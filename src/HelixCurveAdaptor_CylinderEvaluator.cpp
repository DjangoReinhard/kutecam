/* 
 * **************************************************************************
 * 
 *  file:       HelixCurveAdaptor_CylinderEvaluator.cpp
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
#include "HelixCurveAdaptor_CylinderEvaluator.h"


HelixCurveAdaptor_CylinderEvaluator::HelixCurveAdaptor_CylinderEvaluator(const Geom_HelixData& theData)
 : HelixCurveAdaptor::Evaluator(theData) {
  myVCoef = theData.Pitch() * theData.ScaleFactor() / (2 * M_PI);
  }


void HelixCurveAdaptor_CylinderEvaluator::D0(double U, gp_Pnt& P) const {
  double v    = VParameter(U);
  double Rx   = myData.XRadius();
  double Ry   = myData.YRadius();
  double sinU = sin(U);
  double cosU = cos(U);

  P = Rx * cosU * XDir() + Ry * sinU * YDir() + v * ZDir() + Loc();
  }


void HelixCurveAdaptor_CylinderEvaluator::D1(double U, gp_Pnt& P, gp_Vec& V) const {
  double v    = VParameter(U);
  double Rx   = myData.XRadius();
  double Ry   = myData.YRadius();
  double sinU = sin(U);
  double cosU = cos(U);

  P = Rx * cosU * XDir() + Ry * sinU * YDir() + v * ZDir() + Loc();

  double k = myVCoef;

  V = -Rx * sinU * XDir() + Ry * cosU * YDir() + k * ZDir();
  }


void HelixCurveAdaptor_CylinderEvaluator::D2(double U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2) const {
  double v    = VParameter(U);
  double Rx   = myData.XRadius();
  double Ry   = myData.YRadius();
  double sinU = sin(U);
  double cosU = cos(U);

  P = Rx * cosU * XDir() + Ry * sinU * YDir() + v * ZDir() + Loc();

  double k = myVCoef;

  V1 = -Rx * sinU * XDir() + Ry * cosU * YDir() + k * ZDir();
  V2 = -Rx * cosU * XDir() - Ry * sinU * YDir();
  }


void HelixCurveAdaptor_CylinderEvaluator::D3(double , gp_Pnt& , gp_Vec& , gp_Vec& , gp_Vec& ) const {
  Standard_ASSERT_RAISE(0 != 1, "invalid call! - D3 unsupported on Helix");
  }


gp_Vec HelixCurveAdaptor_CylinderEvaluator::DN(double , const Standard_Integer ) const {
  Standard_ASSERT_RAISE(0 != 1, "invalid call! - DN unsupported on Helix");
  return gp_Vec();
  }
