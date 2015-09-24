/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "openglwindow.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>
#include <iostream>

#include <QtGui/QImage>
#include <QCursor>
#include <QtGlobal>
#include <QtCore/qmath.h>

//! [1]
class TriangleWindow : public OpenGLWindow
{
public:
    TriangleWindow();

    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;
    GLfloat* generatePoints(int nb) Q_DECL_OVERRIDE;

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
    GLuint loadShader(GLenum type, const char *source);

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QOpenGLShaderProgram *m_program;
    int m_frame;

    QImage img;

    float x, y, z, ax, ay, az = 0.0f;
};

TriangleWindow::TriangleWindow()
    : m_program(0)
    , m_frame(0)
{
}
//! [1]

//! [2]
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);

    TriangleWindow window;
    window.setFormat(format);
    window.resize(1000, 480);
    window.show();

    window.setAnimating(true);

    return app.exec();
}
//! [2]


//! [3]
static const char *vertexShaderSource =
    "attribute highp vec4 posAttr;\n"
    "attribute lowp vec4 colAttr;\n"
    "varying lowp vec4 col;\n"
    "uniform highp mat4 matrix;\n"
    "void main() {\n"
    "   col = colAttr;\n"
    "   gl_Position = matrix * posAttr;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying lowp vec4 col;\n"
    "void main() {\n"
    "   gl_FragColor = col;\n"
    "}\n";
//! [3]

//! [4]
GLuint TriangleWindow::loadShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

void TriangleWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");

    this->img = QImage(QString("/auto_home/vbazia/Bureau/vbazia/moteur/imagina-gmin317-2015/tp1/heightmap-1.png"));

}
//! [4]

//! [5]
void TriangleWindow::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 16.0f/9.0f, 0.1f, 100.0f);
    matrix.translate(this->x, this->y, this->z);
    matrix.rotate(this->ax, this->ay, this->az, 0);

    m_program->setUniformValue(m_matrixUniform, matrix);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //generations des points et de leurs positions
    int nbpoints = 240;
    GLfloat points[nbpoints*nbpoints*3];

    float interval = 0.05f;
    float maxHeight = 10.0f;
    int cpt = 0;
    for(int i = 0; i < nbpoints; i++){
        for(int j = 0; j < nbpoints; j++){
            points[cpt++] = i * interval;
            points[cpt++] = j * interval;
            QRgb pixColor = this->img.pixel(floor(i*(this->img.width()/nbpoints)), floor(j*(this->img.height()/nbpoints)));
            points[cpt++] = maxHeight * qGray(pixColor) / 255;
        }
    }


    //AFFICHAGE
    cpt = 0;
    bool first = false;
    int last = -1;
    int totake = 0;
    int nbparam = 3;

    for(int k = 0; k < nbpoints - 1; k++){
        last = -1;
        totake = k * nbpoints;
        first = false;
        cpt = 0;
        GLfloat vertices[nbpoints*2*nbparam];
        GLfloat colors[nbpoints*2*3];
        for(int i = 0; i < nbpoints*2; i++){
            if(last != -1){
                if(first){
                    totake = last - nbpoints + 1;
                    first = false;
                }else{
                    totake = last + nbpoints;
                    first = true;
                }
            }
            last = totake;
            vertices[cpt++] = points[totake*3];
            vertices[cpt++] = points[totake*3 + 1];
            vertices[cpt++] = points[totake*3 + 2];
            colors[cpt-3] = 1.0f;
            colors[cpt-2] = 1.0f;
            colors[cpt-1] = 1.0f;
        }
        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, nbpoints*2);
    }

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    m_program->release();

    ++m_frame;
}


bool TriangleWindow::event(QEvent *event)
{
    QMouseEvent *mouseEvent;
    float deltaX = 0.5f;
    float deltaY = 0.5f;

    switch (event->type()) {
    case QEvent::MouseMove:
        mouseEvent = static_cast<QMouseEvent*>(event);
        this->ax -= (deltaX - mouseEvent->x()) * 0.001f;
        this->ay -= (deltaY - mouseEvent->y()) * 0.001f;
        return true;
    }
    OpenGLWindow::event(event);
}
