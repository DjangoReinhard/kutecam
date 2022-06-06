/* 
 * **************************************************************************
 * 
 *  file:       Geom_HelixData.cpp
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
#include "Geom_HelixData.h"
#include "HHelixCurveAdaptor.h"
#include <GeomAbs_Shape.hxx>
#include <GeomConvert_ApproxCurve.hxx>


/*!
 * Uses adaptor classes and invokes GeomConvert_ApproxCurve
 * to approximate with a B-Spline.
 * Created B-Spline is polynomial and is of C2-continuity.
 * Returns true if the B-Spline has been successfully created
 * and false otherwise.
 */
bool Geom_HelixData::MakeHelix(const Geom_HelixData& theSource, Handle(Geom_BSplineCurve)& theTarget) {
  Handle(HHelixCurveAdaptor) anAdaptor = new HHelixCurveAdaptor(theSource);
  double        aTol        = Precision::Confusion();
  GeomAbs_Shape aContinuity = GeomAbs_C2;   /* highest supported continuity */
  int           aMaxSeg     = 10000,        /* max number of spans */
                aMaxDeg     = 9;            /* max degree, consistent with settings in Algo */

  GeomConvert_ApproxCurve anApprox(anAdaptor
                                 , aTol
                                 , aContinuity
                                 , aMaxSeg
                                 , aMaxDeg);
  if (anApprox.HasResult()) theTarget = anApprox.Curve();
  return !theTarget.IsNull();
  }
