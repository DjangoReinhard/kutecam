# kutecam

## Note: work in progress!
 a linux cam solution to create gcode from cad-model

### Requirements
 - Qt5
 - opencascade 7.6
 - cmake build system


## [Wiki](https://github.com/DjangoReinhard/kutecam/wiki)  contains informations about how to build and use kuteCAM

## Screenshot
- **mill contour with open pockets**

 ![first step](sample/kc013.jpg)

## Status
actually working:
- different kinds of selections for drill cycles
- preview with speedcontrol
- mill a clamping plug
- mill plane faces
- mill open pockets / concave contours
- mill vertical faces
- mill circular pockets
- mill cylinder

Postprocessors (implemented as Qt-plugin):
- Fanuc
- Heidenhain
- Sinumeric

Support other languages:
- actually english and german

## Teaser
small video clips for download
- [![common setup](sample/common_Setup.mkv)]
- [![model setup](sample/model_Setup.mkv)]
- [![drill cycles](sample/drill_Cycles.mkv)]
- [![simple sweep](sample/simple_Sweep.mkv)]

### Read more in [kuteCAM Wiki](https://github.com/DjangoReinhard/kutecam/wiki)
