// Copyright (c) 2021 OPEN CASCADE SAS
//
// This file is part of the examples of the Open CASCADE Technology software library.
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

#ifdef _WIN32
  #include <windows.h>
#endif
#include <OpenGl_Context.hxx>
#include "occtviewer.h"
#include "util3d.h"

#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QGridLayout>
#include <QDebug>
#include <Standard_WarningsRestore.hxx>

#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <gp_Circ.hxx>
#include <gp_Quaternion.hxx>
#include <Message.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_BSplineCurve.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OpenGl_FrameBuffer.hxx>


namespace
{
//! Map Qt buttons bitmask to virtual keys.
Aspect_VKeyMouse qtMouseButtons2VKeys(Qt::MouseButtons theButtons) {
  Aspect_VKeyMouse aButtons = Aspect_VKeyMouse_NONE;

  if ((theButtons & Qt::LeftButton) != 0)   aButtons |= Aspect_VKeyMouse_LeftButton;
  if ((theButtons & Qt::MiddleButton) != 0) aButtons |= Aspect_VKeyMouse_MiddleButton;
  if ((theButtons & Qt::RightButton) != 0)  aButtons |= Aspect_VKeyMouse_RightButton;

  return aButtons;
  }


//! Map Qt mouse modifiers bitmask to virtual keys.
Aspect_VKeyFlags qtMouseModifiers2VKeys(Qt::KeyboardModifiers theModifiers) {
  Aspect_VKeyFlags aFlags = Aspect_VKeyFlags_NONE;

  if ((theModifiers & Qt::ShiftModifier) != 0)   aFlags |= Aspect_VKeyFlags_SHIFT;
  if ((theModifiers & Qt::ControlModifier) != 0) aFlags |= Aspect_VKeyFlags_CTRL;
  if ((theModifiers & Qt::AltModifier) != 0)     aFlags |= Aspect_VKeyFlags_ALT;

  return aFlags;
  }
}   // end unnamed namespace


OcctQtViewer::OcctQtViewer(bool verbose, QWidget* theParent)
 : QOpenGLWidget(theParent)
 , myIsCoreProfile(false)
 , verbose(verbose)
 , helper(nullptr) {
  Handle(Aspect_DisplayConnection) aDisp   = new Aspect_DisplayConnection();
  Handle(OpenGl_GraphicDriver)     aDriver = new OpenGl_GraphicDriver(aDisp, false);

  aDriver->ChangeOptions().buffersNoSwap      = true;  // lets QOpenGLWidget to manage buffer swap
  aDriver->ChangeOptions().buffersOpaqueAlpha = true;  // don't write into alpha channel
  aDriver->ChangeOptions().useSystemBuffer    = false; // offscreen FBOs should be always used

  //===================================================================
  // create viewer
  //-------------------------------------------------------------------
  myViewer = new V3d_Viewer(aDriver);
  myViewer->SetDefaultBackgroundColor(Quantity_NOC_GRAY80);
  myViewer->SetDefaultLights();
  myViewer->SetLightOn();

  myContext    = new AIS_InteractiveContext(myViewer);
  myAltContext = new AIS_InteractiveContext(myViewer);

  myViewCube = new AIS_ViewCube();
  myViewCube->SetViewAnimation(myViewAnimation);
  myViewCube->SetFixedAnimationLoop(false);
  myViewCube->SetAutoStartAnimation(true);  
  myViewCube->TransformPersistence()->SetOffset2d(Graphic3d_Vec2i(100, 150));

  //===================================================================
  // note - window will be created later within initializeGL() callback!
  //-------------------------------------------------------------------
  myView = myViewer->CreateView();
  myView->SetImmediateUpdate(false);
  myView->ChangeRenderingParams().NbMsaaSamples  = 4; // warning - affects performance
  myView->ChangeRenderingParams().ToShowStats    = false;
  myView->ChangeRenderingParams().CollectedStats = (Graphic3d_RenderingParams::PerfCounters)
                                                   (Graphic3d_RenderingParams::PerfCounters_FrameRate
                                                  | Graphic3d_RenderingParams::PerfCounters_Triangles);
  configureGrid({0, 0, 1});
  helper = new Util3D();

  // Qt widget setup
  setMouseTracking(true);  
  setBackgroundRole(QPalette::NoRole);  // or NoBackground
  setFocusPolicy(Qt::StrongFocus);      // set focus policy to threat
                                        // QContextMenuEvent from keyboard
  setUpdatesEnabled(true);
  setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

  // OpenGL setup managed by Qt
  QSurfaceFormat aGlFormat;

  aGlFormat.setDepthBufferSize(24);
  aGlFormat.setStencilBufferSize(8);
  //aGlFormat.setOption (QSurfaceFormat::DebugContext, true);
  //aGlFormat.setOption (QSurfaceFormat::DeprecatedFunctions, true);
  aGlFormat.setVersion(4, 5);
  aGlFormat.setProfile(myIsCoreProfile ? QSurfaceFormat::CoreProfile
                                       : QSurfaceFormat::CompatibilityProfile);
#if (QT_VERSION_MAJOR > 5) || (QT_VERSION_MAJOR == 5 && QT_VERSION_MINOR >= 10)
  aGlFormat.setColorSpace(QSurfaceFormat::sRGBColorSpace);
  setTextureFormat(GL_SRGB8_ALPHA8);
#else
  Message::SendWarning("Warning! Qt 5.10+ is required for sRGB setup.\n"
                       "Colors in 3D Viewer might look incorrect (Qt " QT_VERSION_STR " is used).\n");
  aDriver->ChangeOptions().sRGBDisable = true;
#endif
  setFormat (aGlFormat);

#if defined(_WIN32)
  // never use ANGLE on Windows, since OCCT 3D Viewer does not expect this
  QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
  //QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
#endif
  Handle(Prs3d_Drawer) hlStyle = myContext->HighlightStyle();

  hlStyle->SetMethod(Aspect_TOHM_COLOR);
  hlStyle->SetColor(Quantity_NOC_BLUE);
  hlStyle->SetDisplayMode(1);
  hlStyle->SetTransparency(0.2f);
  myContext->SetHighlightStyle(hlStyle);
  }


OcctQtViewer::~OcctQtViewer() {
  }


Handle(AIS_Shape) OcctQtViewer::baseFace() {
  return myBaseFace;
  }


void OcctQtViewer::configureGrid(const gp_Dir& d) {
  if (!myView) return;
  gp_Ax3 Ax3({0, 0, 0}, d);
  double scale = myView->Scale();

  myViewer->SetPrivilegedPlane(Ax3);
  if      (scale < 3)  myViewer->SetRectangularGridValues(0, 0, 10, 10, 0);
  else if (scale < 6)  myViewer->SetRectangularGridValues(0, 0, 5, 5, 0);
  else if (scale < 55) myViewer->SetRectangularGridValues(0, 0, 1, 1, 0);
  else                 myViewer->SetRectangularGridValues(0, 0, 0.1, 0.1, 0);
  myViewer->SetRectangularGridGraphicValues(900, 900, 0);
  myViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
  }


void OcctQtViewer::changeGrid() {
  double scale = myView->Scale();

  if      (scale < 3)  myViewer->SetRectangularGridValues(0, 0, 10, 10, 0);
  else if (scale < 6)  myViewer->SetRectangularGridValues(0, 0, 5, 5, 0);
  else if (scale < 55) myViewer->SetRectangularGridValues(0, 0, 1, 1, 0);
  else                 myViewer->SetRectangularGridValues(0, 0, 0.1, 0.1, 0);
  }


void OcctQtViewer::clipPlane(bool clipX, bool clipY) {
  if (clipX || clipY) {
     gp_Pln cutPlane({0, 0, 0}, {-1, 0, 0});

     if (clipY) cutPlane = gp_Pln({0, 0, 0}, {0, 1, 0});
     myClipPlane = new Graphic3d_ClipPlane();
     myClipPlane->SetEquation(cutPlane);
     myClipPlane->SetCapping(true);

     // set the material with red color of clipping plane
     Graphic3d_MaterialAspect aMat = myClipPlane->CappingMaterial();

     aMat.SetAmbientColor(Quantity_NOC_RED);
     aMat.SetDiffuseColor(Quantity_NOC_RED);
     aMat.SetTransparency(0.8);
     myClipPlane->SetCappingMaterial(aMat);

     view()->AddClipPlane(myClipPlane);
     myClipPlane->SetOn(true);
     }
  refresh();
  }


void OcctQtViewer::unClip() {
  if (!myClipPlane.IsNull()) view()->RemoveClipPlane(myClipPlane);
  refresh();
  }


void OcctQtViewer::closeEvent(QCloseEvent* e) {
  qDebug() << "OcctQtViewer::closeEvent?!?";
  e->accept();
  }


void OcctQtViewer::dumpGlInfo(bool theIsBasic) {
  TColStd_IndexedDataMapOfStringString aGlCapsDict;

  myView->DiagnosticInformation(aGlCapsDict
                              , theIsBasic ? Graphic3d_DiagnosticInfo_Basic
                                           : Graphic3d_DiagnosticInfo_Complete);
  TCollection_AsciiString anInfo;

  for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter (aGlCapsDict)
     ; aValueIter.More()
     ; aValueIter.Next()) {
      if (!aValueIter.Value().IsEmpty()) {
         if (!anInfo.IsEmpty()) {
            anInfo += "\n";
            }
         anInfo += aValueIter.Key() + ": " + aValueIter.Value();
         }
      }
  Message::SendInfo(anInfo);
  myGlInfo = QString::fromUtf8(anInfo.ToCString());
  }


//void OcctQtViewer::evaluateSelection() {
//  qDebug() << "OcctQtViewer::evaluateSelection()";
//  emit clearCurves();

//  myContext->InitSelected();
//  while (myContext->MoreSelected())  {
//        if (myContext->HasSelectedShape()) {
//           TopoDS_Shape shape = myContext->SelectedShape();

//           emit shapeSelected(shape);
//           }
//        else {
//           Handle(AIS_InteractiveObject) anyObj = myContext->DetectedInteractive();
////TODO:
//           qDebug() << "no selected shape found!";
//           }
//        myContext->NextSelected();
//        }
//  }


std::vector<TopoDS_Shape> OcctQtViewer::selection() {
  std::vector<TopoDS_Shape> rv;

  myContext->InitSelected();
  while (myContext->MoreSelected())  {
        if (myContext->HasSelectedShape()) {
           TopoDS_Shape shape = myContext->SelectedShape();

           rv.push_back(shape);
           }
        myContext->NextSelected();
        }
  return rv;
  }


void OcctQtViewer::fitAll() {
  myView->FitAll(myBounds, false);
  refresh();
  }


void OcctQtViewer::refresh() {
  myView->Invalidate();
  changeGrid();
  update();
  }


void OcctQtViewer::genCircle(Handle(Geom_Circle) c, double first, double last) {
  const gp_Ax2& axis  = c->Position();
  gp_Pnt        center(axis.Location().X(), axis.Location().Y(), axis.Location().Z());
  gp_Dir        dir(axis.Direction());
  gp_Dir        xDir(axis.XDirection());
  gp_Dir        yDir(axis.YDirection());
  double        r = c->Radius();
  double        step  = (last - first) / 10;
  double        param = first;
  double        cp, sp;
  gp_Pnt        pos;

  if (dots.count() > 0) {
     removeShapes(dots);
     dots.clear();
     }
  qDebug() << "\tparam:" << param << "\tstep:" << step;
  for (int i=0; i < 11; ++i, param += step) {
      qDebug() << "\tparam:" << param;
      cp = cos(param);
      sp = sin(param);

      pos.SetX(center.X() + r * cp * xDir.X() + r * sp * yDir.X());
      pos.SetY(center.Y() + r * cp * xDir.Y() + r * sp * yDir.Y());
      pos.SetZ(center.Z() + r * cp * xDir.Z() + r * sp * yDir.Z());

      if (!i)           createAxisCross(pos, 2, &dots, Quantity_NOC_YELLOW);
      else if (i == 10) createAxisCross(pos, 2, &dots, Quantity_NOC_BLACK);
      else              createAxisCross(pos, 2, &dots, Quantity_NOC_WHITE);
      }
  showShapes(dots);
  refresh();
  }


// P(U) = O + MajorRad * Cos(U) * XDir + MinorRad * Sin(U) * YDir
// O - origin
void OcctQtViewer::genEllipse(Handle(Geom_Ellipse) e, double first, double last) {
  const gp_Dir& xDir   = e->XAxis().Direction();
  const gp_Dir& yDir   = e->YAxis().Direction();
  double        r      = e->MinorRadius();
  double        R      = e->MajorRadius();
  gp_Pnt        center = e->Location();
  double        step   = (last - first) / 10;
  double        param  = first;
  double        cp, sp;
  gp_Pnt        pos;

  if (dots.count() > 0) {
     removeShapes(dots);
     dots.clear();
     }
  qDebug() << "\tparam:" << param << "\tstep:" << step;
  for (int i=0; i < 11; ++i, param += step) {
      qDebug() << "\tparam:" << param;
      cp = cos(param);
      sp = sin(param);

      pos.SetX(center.X() + R * cp * xDir.X() + r * sp * yDir.X());
      pos.SetY(center.Y() + R * cp * xDir.Y() + r * sp * yDir.Y());
      pos.SetZ(center.Z() + R * cp * xDir.Z() + r * sp * yDir.Z());

      if (!i)           createAxisCross(pos, 2, &dots, Quantity_NOC_YELLOW);
      else if (i == 10) createAxisCross(pos, 2, &dots, Quantity_NOC_BLACK);
      else              createAxisCross(pos, 2, &dots);
      }
  showShapes(dots);
  refresh();
  }


void OcctQtViewer::genLine(Handle(Geom_Line) line, double first, double last) {
  const gp_Ax1& axis  = line->Position();
  gp_Pnt        start(axis.Location().X(), axis.Location().Y(), axis.Location().Z());
  gp_Dir        dir(axis.Direction().X(), axis.Direction().Y(), axis.Direction().Z());
  double        step  = (last - first) / 10;
  double        param = first;
  gp_Pnt        pos   = start;

  if (dots.count() > 0) {
     removeShapes(dots);
     dots.clear();
     }
  qDebug() << "\tparam:" << param << "\tstep:" << step;
  for (int i=0; i < 11; ++i, param += step) {
      qDebug() << "\tparam:" << param;

      pos.SetX(start.X() + param * dir.X());
      pos.SetY(start.Y() + param * dir.Y());
      pos.SetZ(start.Z() + param * dir.Z());

      if (!i)           createAxisCross(pos, 2, &dots, Quantity_NOC_YELLOW);
      else if (i == 10) createAxisCross(pos, 2, &dots, Quantity_NOC_BLACK);
      else              createAxisCross(pos, 2, &dots);
      }
  showShapes(dots);
  refresh();
  }


const QString& OcctQtViewer::getGlInfo() {
  if (myGlInfo.isEmpty()) dumpGlInfo(true);
  return myGlInfo;
  }


void OcctQtViewer::initializeGL() {
  const QRect            aRect = rect();
  const Graphic3d_Vec2i  aViewSize(aRect.right()  - aRect.left()
                                 , aRect.bottom() - aRect.top());
  qDebug() << "OcctQtViewer::initializeGL() ... viewSize: " << aRect;

  Handle(OpenGl_Context) aGlCtx = new OpenGl_Context();

  if (!aGlCtx->Init(myIsCoreProfile)) {
     Message::SendFail() << "Error: OpenGl_Context is unable to wrap OpenGL context";
     QMessageBox::critical(0, "Failure", "OpenGl_Context is unable to wrap OpenGL context");
     QApplication::exit(1);

     return;
     }
  Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());

  if (!aWindow.IsNull()) {
     aWindow->SetSize(aViewSize.x(), aViewSize.y());
     myView->SetWindow(aWindow, aGlCtx->RenderingContext());
     if (verbose) dumpGlInfo(true);
     }
  else {
     aWindow = new Aspect_NeutralWindow();

     Aspect_Drawable aNativeWin = (Aspect_Drawable )winId();
#ifdef _WIN32
     //HGLRC aWglCtx    = wglGetCurrentContext();
     HDC   aWglDevCtx = wglGetCurrentDC();
     HWND  aWglWin    = WindowFromDC(aWglDevCtx);

     aNativeWin       = (Aspect_Drawable)aWglWin;
#endif
     aWindow->SetNativeHandle(aNativeWin);
     aWindow->SetSize(aViewSize.x(), aViewSize.y());
     myView->SetWindow(aWindow, aGlCtx->RenderingContext());
     if (verbose) dumpGlInfo(true);
     }
  SetLockOrbitZUp(true);
  myViewer->SetDefaultLights();
  myViewer->SetLightOn();
  myContext->SetDisplayMode(AIS_Shaded, Standard_True);

  Handle(Prs3d_Drawer) t_hilight_style = myContext->HighlightStyle();

  t_hilight_style->SetMethod(Aspect_TOHM_COLOR);
  t_hilight_style->SetColor(Quantity_NOC_MAGENTA);
  t_hilight_style->SetDisplayMode(1);
  t_hilight_style->SetTransparency(0.1f);
  Handle(Prs3d_Drawer)  t_select_style = myContext->SelectionStyle();

  t_select_style->SetMethod(Aspect_TOHM_COLOR);
  t_select_style->SetColor(Quantity_NOC_YELLOW);
  t_select_style->SetDisplayMode(1);
  t_select_style->SetTransparency(0.5f);
  reset3D();
  }


TopLoc_Location OcctQtViewer::getLocation(const opencascade::handle<AIS_Shape> &s) {
  return myContext->Location(s);
  }


void OcctQtViewer::mousePressEvent(QMouseEvent* e) {
  QOpenGLWidget::mousePressEvent(e);
  const Graphic3d_Vec2i  aPnt(e->pos().x(), e->pos().y());
  const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(e->modifiers());

  if (!myView.IsNull()
    && UpdateMouseButtons(aPnt
                        , qtMouseButtons2VKeys(e->buttons())
                        , aFlags
                        , false)) {
     updateView();
     }
  }


void OcctQtViewer::mouseReleaseEvent(QMouseEvent* e) {
  QOpenGLWidget::mouseReleaseEvent(e);
  const Graphic3d_Vec2i  aPnt(e->pos().x(), e->pos().y());
  const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(e->modifiers());

  if (!myView.IsNull()
    && UpdateMouseButtons(aPnt
                        , qtMouseButtons2VKeys(e->buttons())
                        , aFlags
                        , false)) {
     updateView();
     }
  }


void OcctQtViewer::mouseMoveEvent(QMouseEvent* e) {
  QOpenGLWidget::mouseMoveEvent(e);
  const Graphic3d_Vec2i aNewPos (e->pos().x(), e->pos().y());

  if (!myView.IsNull()
    && UpdateMousePosition(aNewPos
                         , qtMouseButtons2VKeys(e->buttons())
                         , qtMouseModifiers2VKeys(e->modifiers())
                         , false)) {
     updateView();
     }
  }


void OcctQtViewer::reset2D() {
  myContext->RemoveAll(false);
  createAxisCross(gp_Pnt());
  }


void OcctQtViewer::reset3D() {
  myContext->RemoveAll(false);
  myContext->Display(myViewCube, false);
  createAxisCross(gp_Pnt(0, 0, 0.005));
  }


void OcctQtViewer::createAxisCross(const gp_Pnt& p, double lineLen, QVector<Handle(AIS_Shape)>* pV, Quantity_Color c) {
  TopoDS_Edge       edge   = BRepBuilderAPI_MakeEdge({p.X() - lineLen, p.Y(), p.Z()}
                                                   , {p.X() + lineLen, p.Y(), p.Z()});
  Handle(AIS_Shape) sXAxis = new AIS_Shape(edge);

  sXAxis->SetColor(Quantity_NOC_RED);
  sXAxis->SetWidth(2);
  edge = BRepBuilderAPI_MakeEdge({p.X(), p.Y() - lineLen, p.Z()}
                               , {p.X(), p.Y() + lineLen, p.Z()});
  Handle(AIS_Shape) sYAxis = new AIS_Shape(edge);

  sYAxis->SetColor(Quantity_NOC_GREEN);
  sYAxis->SetWidth(2);
  edge= BRepBuilderAPI_MakeEdge({p.X(), p.Y(), p.Z() - lineLen}
                              , {p.X(), p.Y(), p.Z() + lineLen});
  Handle(AIS_Shape) sZAxis = new AIS_Shape(edge);

  sZAxis->SetColor(Quantity_NOC_BLUE);
  sZAxis->SetWidth(2);
  if (pV) {
     sXAxis->SetColor(c);
     sYAxis->SetColor(c);
     sZAxis->SetColor(c);
     pV->append(sXAxis);
     pV->append(sYAxis);
     pV->append(sZAxis);
     }
  else {
     if (!c.IsEqual(Quantity_NOC_GRAY)) {
        sXAxis->SetColor(c);
        sYAxis->SetColor(c);
        sZAxis->SetColor(c);
        }
     myAltContext->Display(sXAxis, 1, -1, false);
     myAltContext->Display(sYAxis, 1, -1, false);
     myAltContext->Display(sZAxis, 1, -1, false);
     }
  }


void OcctQtViewer::setBounds(const Bnd_Box &bounds) {
  myBounds = bounds;
  }


void OcctQtViewer::showShape(Handle(AIS_Shape) s, bool selectable) {
  shapes3D.append(s);
  context()->Display(s
                   , 1 // displayMode
                   , selectable ? 0 : -1 // selectionMode
                   , false);
  refresh();
  }


void OcctQtViewer::showShapes(const std::vector<Handle(AIS_Shape)>& v, bool selectable) {
  if (!v.size()) return;
  for (auto& s : v) showShape(s, selectable);
  }


void OcctQtViewer::showShapes(const QVector<Handle(AIS_Shape)>& v, bool selectable) {
  if (!v.size()) return;
  for (auto& s : v) showShape(s, selectable);
  }


void OcctQtViewer::updateView() {
  changeGrid();
  update();
  }


void OcctQtViewer::wheelEvent(QWheelEvent* e) {
  QOpenGLWidget::wheelEvent(e);
  const Graphic3d_Vec2i aPos(e->position().x(), e->position().y());

  if (!myView.IsNull()
   && UpdateZoom(Aspect_ScrollDelta(aPos, e->angleDelta().y() / 10))) {
     updateView();
     }  
  }


void OcctQtViewer::paintGL() {
  if (myView->Window().IsNull()) return;

  // wrap FBO created by QOpenGLWidget
  Handle(OpenGl_GraphicDriver)  aDriver     = Handle(OpenGl_GraphicDriver)::DownCast(myContext->CurrentViewer()->Driver());
  const Handle(OpenGl_Context)& aGlCtx      = aDriver->GetSharedContext();
  Handle(OpenGl_FrameBuffer)    aDefaultFbo = aGlCtx->DefaultFrameBuffer();

  if (aDefaultFbo.IsNull()) {
     aDefaultFbo = new OpenGl_FrameBuffer();
     aGlCtx->SetDefaultFrameBuffer(aDefaultFbo);
     }
  if (!aDefaultFbo->InitWrapper(aGlCtx)) {
     aDefaultFbo.Nullify();
     Message::DefaultMessenger()->Send("Default FBO wrapper creation failed", Message_Fail);
     QMessageBox::critical(0, "Failure", "Default FBO wrapper creation failed");
     QApplication::exit(1);
     return;
     }
  Graphic3d_Vec2i              aViewSizeOld;
  Graphic3d_Vec2i              aViewSizeNew = aDefaultFbo->GetVPSize();
  Handle(Aspect_NeutralWindow) aWindow      = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());

  aWindow->Size(aViewSizeOld.x(), aViewSizeOld.y());
  if (aViewSizeNew != aViewSizeOld) {
     aWindow->SetSize(aViewSizeNew.x(), aViewSizeNew.y());
     myView->MustBeResized();
     myView->Invalidate();
     }
  // flush pending input events and redraw the viewer
  myView->InvalidateImmediate();
  FlushViewEvents(myContext, myView, true);
  }


void OcctQtViewer::handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx
                                  , const Handle(V3d_View)& theView) {
  AIS_ViewController::handleViewRedraw(theCtx, theView);

  if (myToAskNextFrame) {
     updateView();      // ask more frames for animation
     }
  }


void OcctQtViewer::OnObjectDragged(const opencascade::handle<AIS_InteractiveContext> &theCtx, const opencascade::handle<V3d_View> &theView, AIS_DragAction theAction) {
//  qDebug() << "object dragged ...";
  configureGrid({0, 0, 1});
  updateView();
  }


//void OcctQtViewer::OnSelectionChanged(const Handle(AIS_InteractiveContext)& theCtx, const Handle(V3d_View)& theView) {
//  evaluateSelection();
//  }


Handle(AIS_Shape) OcctQtViewer::mainShape() {
  return myMainShape;
  }


void OcctQtViewer::setMainShape(opencascade::handle<AIS_Shape> s) {
  myMainShape = s;
  }


void OcctQtViewer::frontView() {
  myView->SetProj(V3d_Yneg);
  configureGrid({0, 1, 0});
  fitAll();
  }


void OcctQtViewer::backView() {
  myView->SetProj(V3d_Ypos);
  configureGrid({0, 1, 0});
  fitAll();
  }


void OcctQtViewer::bottomView() {
  myView->SetProj(V3d_Zneg);
  configureGrid({0, 0, 1});
  fitAll();
  }


void OcctQtViewer::topView() {
  myView->SetProj(V3d_Zpos);
  configureGrid({0, 0, 1});
  fitAll();
  }


void OcctQtViewer::leftView() {
  myView->SetProj(V3d_Xneg);
  configureGrid({1, 0, 0});
  fitAll();
  }


void OcctQtViewer::rightView() {
  myView->SetProj(V3d_Xpos);
  configureGrid({1, 0, 0});
  fitAll();
  }


void OcctQtViewer::iso1View() {
  myView->SetProj(V3d_XposYnegZpos);
  configureGrid({0, 0, 1});
  fitAll();
  }


void OcctQtViewer::iso2View() {
  myView->SetProj(V3d_XposYposZpos);
  configureGrid({0, 0, 1});
  fitAll();
  }


void OcctQtViewer::iso3View() {
  myView->SetProj(V3d_XnegYposZpos);
  configureGrid({0, 0, 1});
  fitAll();
  }


void OcctQtViewer::iso4View() {
  myView->SetProj(V3d_XnegYnegZpos);
  configureGrid({0, 0, 1});
  fitAll();
  }


void OcctQtViewer::removeShape(Handle(AIS_Shape) s, bool updateView) {
  context()->Remove(s, updateView);
  }


void OcctQtViewer::removeShapes(QVector<opencascade::handle<AIS_Shape>>& v) {
  if (!v.size()) return;
  for (auto& s : v)
      removeShape(s);      
  }


void OcctQtViewer::removeShapes(const std::vector<Handle(AIS_Shape)>& v) {
  if (!v.size()) return;
  for (auto& s : v)
      removeShape(s);
  }


void OcctQtViewer::move(double dX, double dY, double dZ) {
  qDebug() << "OcctQtViewer::move(" << dX << "/" << dY << "/" << dZ << ")";
  gp_Trsf               move;

  move.SetTranslation({0, 0, 0}, {dX, dY, dZ});
  TopLoc_Location       tll(move);
  AIS_ListOfInteractive shapes;


  myContext->DisplayedObjects(shapes);
  for (Handle(AIS_InteractiveObject)& s : shapes) {
      qDebug() << s->get_type_name();
      if (s->DynamicType() == STANDARD_TYPE(AIS_ViewCube)) continue;
      myContext->SetLocation(s, tll);
      }
  myView->Invalidate();
  update();
  }


void OcctQtViewer::setWorkpiece(opencascade::handle<AIS_Shape> wp) {
  myWorkpiece = wp;
  }


void OcctQtViewer::setBaseFace(opencascade::handle<AIS_Shape> bf) {
  myBaseFace = bf;
  }


void OcctQtViewer::rotate(double dA, double dB, double dC) {
  gp_Quaternion q;
  gp_Trsf       r;

  q.SetEulerAngles(gp_Intrinsic_XYZ, dA, dB, dC);
  r.SetRotation(q);
  TopLoc_Location       tll(r);
  AIS_ListOfInteractive shapes;

  myContext->DisplayedObjects(shapes);
  for (Handle(AIS_InteractiveObject)& s : shapes) {
      if (s->DynamicType() == STANDARD_TYPE(AIS_ViewCube)) continue;
      if (s == myWorkpiece)     myWorkpiece->SetLocalTransformation(tll);
      else if (s == myBaseFace) myBaseFace->SetLocalTransformation(tll);
      else                      myContext->SetLocation(s, tll);
      }
  myView->Invalidate();
  update();
  }


void  OcctQtViewer::switchOrthographic(const QVariant& ortho) {
  if (ortho.toBool())
     myView->Camera()->SetProjectionType(Graphic3d_Camera::Projection_Orthographic);
  else
     myView->Camera()->SetProjectionType(Graphic3d_Camera::Projection_Perspective);
  fitAll();
  }


void  OcctQtViewer::switchWireframe(const QVariant& wf) {
  myContext->SetDisplayMode(wf.toBool() ? AIS_WireFrame : AIS_Shaded, true);
  }
