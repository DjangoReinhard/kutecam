// Copyright (c) 2021 OPEN CASCADE SAS
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
//
// changed and extended by Django Reinhard
//
#ifndef _OcctQtViewer_HeaderFile
#define _OcctQtViewer_HeaderFile

#include <Standard_WarningsDisable.hxx>
#include <QOpenGLWidget>
#include <QVariant>
#include <Standard_WarningsRestore.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>

#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewController.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <V3d_View.hxx>

class AIS_ViewCube;
class Util3D;


//! OCCT 3D View.
class OcctQtViewer : public QOpenGLWidget, public AIS_ViewController
{
  Q_OBJECT

public:
  OcctQtViewer(bool verbose = false, QWidget* theParent = nullptr);
  virtual ~OcctQtViewer();

  const QString& getGlInfo();
  virtual QSize  minimumSizeHint() const override { return QSize(200, 200); }
  virtual QSize  sizeHint()        const override { return QSize(500, 400); }
  Handle(AIS_Shape) baseFace();
  Handle(AIS_Shape) mainShape();
  Handle(AIS_Shape) workpiece();
  TopLoc_Location   getLocation(const Handle(AIS_Shape)& s);
  std::vector<TopoDS_Shape> selection();

  const Handle(AIS_InteractiveContext)& context() const { return myContext; }
  Handle(AIS_InteractiveContext)        context()       { return myContext; }
  void createAxisCross(const gp_Pnt& /* = gp_Pnt() */, double lineLen = 500, QVector<Handle(AIS_Shape)>* pV = nullptr, Quantity_Color c = Quantity_NOC_GRAY);
  void genLine(Handle(Geom_Line) l, double first, double last);
  void genCircle(Handle(Geom_Circle) l, double first, double last);
  void genEllipse(Handle(Geom_Ellipse), double first, double last);

  void move(double dX, double dY, double dZ);
  void rotate(double dA, double dB, double dC);
  void setBaseFace(Handle(AIS_Shape) bf);
  void setBounds(const Bnd_Box& bounds);
  void setMainShape(Handle(AIS_Shape) s);
  void setWorkpiece(Handle(AIS_Shape) wp);

  void showShape(Handle(AIS_Shape) s, bool selectable = true);
  void showShapes(const QVector<Handle(AIS_Shape)>& v, bool selectable = true);
  void showShapes(const std::vector<Handle(AIS_Shape)>& v, bool selectable = true);
  void removeShape(Handle(AIS_Shape) s, bool updateView = false);
  void removeShapes(QVector<Handle(AIS_Shape)>& v);
  void removeShapes(const std::vector<Handle(AIS_Shape)>& v);

public slots:
  void clipPlane(bool clipX, bool clipY);
  void fitAll();
  void frontView();
  void backView();
  void bottomView();
  void topView();
  void leftView();
  void rightView();
  void iso1View();
  void iso2View();
  void iso3View();
  void iso4View();
  void reset2D();
  void reset3D();
  void refresh();
//  virtual void OnSelectionChanged (const Handle(AIS_InteractiveContext)& theCtx, const Handle(V3d_View)& theView) override;
  virtual void OnObjectDragged(const opencascade::handle<AIS_InteractiveContext> &theCtx, const opencascade::handle<V3d_View> &theView, AIS_DragAction theAction) override;
  void switchOrthographic(const QVariant& ortho);
  void switchWireframe(const QVariant& wf);
  void unClip();

signals:
  void clearCurves();
  void shapeSelected(const TopoDS_Shape& shape);

protected:
  // OpenGL events
  virtual void initializeGL() override;
  virtual void paintGL() override;  

  // user input events
  virtual void closeEvent(QCloseEvent* e) override;
  virtual void mousePressEvent(QMouseEvent* e) override;
  virtual void mouseReleaseEvent(QMouseEvent* e) override;
  virtual void mouseMoveEvent(QMouseEvent* e) override;
  virtual void wheelEvent(QWheelEvent* e) override;
//  virtual void evaluateSelection();

  const Handle(V3d_Viewer)&             viewer() const    { return myViewer; }
  const Handle(V3d_View)&               view() const      { return myView; }
  Handle(AIS_ViewCube)                  cube() const      { return myViewCube; }
  Handle(AIS_Shape)                     cone()            { return myCone; }

private:
  void configureGrid(const gp_Dir& d);
  void changeGrid();
  void dumpGlInfo(bool theIsBasic);
  void updateView();            //! Request widget paintGL() event.

  virtual void handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx
                              , const Handle(V3d_View)& theView) override;

private:
  Handle(V3d_Viewer)             myViewer;
  Handle(V3d_View)               myView;
  Handle(Graphic3d_ClipPlane)    myClipPlane;
  Handle(AIS_InteractiveContext) myContext;
  Handle(AIS_InteractiveContext) myAltContext;
  Handle(AIS_Shape)              myCone;
  Handle(AIS_ViewCube)           myViewCube;
  Handle(AIS_Shape)              myMainShape;
  Handle(AIS_Shape)              myWorkpiece;
  Handle(AIS_Shape)              myBaseFace;
  QVector<Handle(AIS_Shape)>     shapes3D;
  QVector<Handle(AIS_Shape)>     dots;
  Util3D*                        helper;
  Bnd_Box                        myBounds;
  QString                        myGlInfo;
  bool                           myIsCoreProfile;
  bool                           verbose;
  };
#endif // _OcctQtViewer_HeaderFile
