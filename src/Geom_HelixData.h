/* 
 * **************************************************************************
 * 
 *  file:       Geom_HelixData.h
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
#ifndef GEOM_HELIXDATA_H
#define GEOM_HELIXDATA_H
#include <gp_Ax3.hxx>
#include <Geom_BSplineCurve.hxx>


//! Defines data elements that can be reused for both common
//  ACIS 'helix' and ASM 'helix_int_cur'.
class Geom_HelixData
{
public:
  Geom_HelixData()
   : myXRadius(0.)
   , myYRadius(0.)
   , myPitch(0.)
   , myTaper(0.)
   , myRangeMin(0.)
   , myRangeMax(2 * M_PI)
   , myScaleFactor(1.) {
    }

  bool MakeHelix(const Geom_HelixData& theSource, Handle(Geom_BSplineCurve)& theTarget);

  void setPosition(const gp_Ax3& pos) { myPosition = pos; }
  void setRadius(double r)            { myXRadius = r; myYRadius = r; }
  void setRangeMax(double r)          { myRangeMax = r; }
  void setPitch(double p)             { myPitch = p; }
  void setScaleFactor(double f)       { myScaleFactor = f; }

  const gp_Ax3& Position() const { return myPosition; }
  double XRadius() const         { return myXRadius; }
  double YRadius() const         { return myYRadius; }
  double Pitch() const           { return myPitch; }
  double Taper() const           { return myTaper; }
  double RangeMin() const        { return myRangeMin; }
  double RangeMax() const        { return myRangeMax; }
  double ScaleFactor() const     { return myScaleFactor; }

private:
  gp_Ax3 myPosition; // can be right- or left-handed
  double myXRadius;
  double myYRadius;
  double myPitch;    // must be >= 0, if = 0 then is planar
  double myTaper;    // if > 0, helix widens along the Z-axis, if 0 - then lies on a cylinder
  double myRangeMin;
  double myRangeMax;
  double myScaleFactor;
  };
#endif
