/* 
 * **************************************************************************
 * 
 *  file:       HelixCurveAdaptor_p.h
 *  project:    kuteCAM
 *  subproject: main application
 *  purpose:    create a graphical application, that assists in identify
 *              and process model elements
 *  created:    19.1.2022 by Django Reinhard
 *  copyright:  (c) 2022 Django Reinhard -  all rights reserved
 * 
 *  This program is free software: you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by 
 *  the Free Software Foundation, either version 2 of the License, or 
 *  (at your option) any later version. 
 *   
 *  This program is distributed in the hope that it will be useful, 
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *  GNU General Public License for more details. 
 *   
 *  You should have received a copy of the GNU General Public License 
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * **************************************************************************
 */
#ifndef HELIXCURVEADAPTOR_P_H
#define HELIXCURVEADAPTOR_P_H
#include "HelixCurveAdaptor.h"
#include "Geom_HelixData.h"
#include <gp_Pnt.hxx>


/*!
 * \class HelixCurveAdaptor::Evaluator
 * \brief Base abstract class to evaluate helix.
 */
class HelixCurveAdaptor::Evaluator
{
public:
  Evaluator(const Geom_HelixData& theData)
   : myData(theData)
   , myVCoef(1.) {
    }
  virtual ~Evaluator() {}

  const Geom_HelixData& Data() const  { return myData; }
  double         VParameter(double U) const { return U * myVCoef; }
  virtual void   D0(double U, gp_Pnt& P) const = 0;
  virtual void   D1(double U, gp_Pnt& P, gp_Vec& V) const = 0;
  virtual void   D2(double U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2) const = 0;
  virtual void   D3(double U, gp_Pnt& P, gp_Vec& V1, gp_Vec& V2, gp_Vec& V3) const = 0;
  virtual gp_Vec DN(double U, Standard_Integer N) const = 0;

protected:
  const gp_XYZ& Loc()  const { return myData.Position().Location().XYZ(); }
  const gp_XYZ& XDir() const { return myData.Position().XDirection().XYZ(); }
  const gp_XYZ& YDir() const { return myData.Position().YDirection().XYZ(); }
  const gp_XYZ& ZDir() const { return myData.Position().Direction().XYZ(); }

  Geom_HelixData myData;
  double         myVCoef; //coefficient to multiply U to get a V parameter on a respective surface
  };
#endif
