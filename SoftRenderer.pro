#-------------------------------------------------
#
# Project created by QtCreator 2019-04-28T14:58:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SoftRenderer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        Window.cpp \
    SoftRenderer/Math/Vector2D.cpp \
    SoftRenderer/Math/Vector3D.cpp \
    SoftRenderer/Math/Vector4D.cpp \
    SoftRenderer/Math/Matrix4x4.cpp \
    RenderLoop.cpp \
    SoftRenderer/Math/Quaternion.cpp \
    SoftRenderer/Camera/Camera3D.cpp \
    SoftRenderer/Camera/FPSCamera.cpp \
    SoftRenderer/Camera/TPSCamera.cpp \
    SoftRenderer/Camera/Transform3D.cpp \
    SoftRenderer/Pipeline/FrameBuffer.cpp \
    SoftRenderer/Pipeline/Light.cpp \
    SoftRenderer/Pipeline/Mesh.cpp \
    SoftRenderer/Pipeline/ObjModel.cpp \
    SoftRenderer/Pipeline/Pipeline.cpp \
    SoftRenderer/Pipeline/Texture2D.cpp \
    SoftRenderer/Shader/BaseShader.cpp \
    SoftRenderer/Shader/GouraudShader.cpp \
    SoftRenderer/Shader/PhongShader.cpp \
    SoftRenderer/Shader/SimpleShader.cpp

HEADERS += \
        Window.h \
    SoftRenderer/Math/MathUtils.h \
    SoftRenderer/Math/Vector2D.h \
    SoftRenderer/Math/Vector3D.h \
    SoftRenderer/Math/Vector4D.h \
    SoftRenderer/Math/Matrix4x4.h \
    RenderLoop.h \
    SoftRenderer/Math/Quaternion.h \
    SoftRenderer/Camera/Camera3D.h \
    SoftRenderer/Camera/FPSCamera.h \
    SoftRenderer/Camera/TPSCamera.h \
    SoftRenderer/Camera/Transform3D.h \
    SoftRenderer/Pipeline/FrameBuffer.h \
    SoftRenderer/Pipeline/Light.h \
    SoftRenderer/Pipeline/Mesh.h \
    SoftRenderer/Pipeline/ObjModel.h \
    SoftRenderer/Pipeline/Pipeline.h \
    SoftRenderer/Pipeline/Texture2D.h \
    SoftRenderer/Shader/BaseShader.h \
    SoftRenderer/Shader/GouraudShader.h \
    SoftRenderer/Shader/PhongShader.h \
    SoftRenderer/Shader/SimpleShader.h

FORMS += \
        Window.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/include
