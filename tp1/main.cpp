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

#include <QtCore/qmath.h>
#include <stdlib.h>
#include <time.h>
#include <QMouseEvent>
#include <iostream>

using namespace std;

#define nbSommets 240
GLfloat points[nbSommets*nbSommets*3];
//GLfloat color[nbSommets*nbSommets*3];

//! [1]
class TriangleWindow : public OpenGLWindow
{
public:
    TriangleWindow();

    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;

private:
    GLuint loadShader(GLenum type, const char *source);

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QOpenGLShaderProgram *m_program;
    int m_frame;

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
    window.resize(640, 480);
    window.show();

    //window.setAnimating(true);

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

    glEnable(GL_FOG);
    glClearColor(0.0f, 0.0f, 0.0f, 1);
    //glEnable( GL_LIGHTING );

}
//! [4]

//! [5]
void TriangleWindow::render()
{
    QImage image ("C:/Users/Galaks/Desktop/tp1_Moteur/tp1/heightmap-1.png");


    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 4.0f/3.0f, 0.1f, 1000.0f);
    matrix.translate(m_x, m_y, m_z);


    matrix.rotate(m_xrotation, 1, 0, 0);
    matrix.rotate(m_zrotation, 0, 0, 1);

    m_program->setUniformValue(m_matrixUniform, matrix);

    GLfloat x = 0.0f;
    GLfloat y = (float)nbSommets/2.0f;
    GLfloat z = 0.0f;

    GLfloat sommets[nbSommets*2*3];
    GLfloat color[nbSommets*2*3];


    if(!m_drawn)
    {
        srand(time(NULL));

        int indice = 0;
        for(int i = 0; i < nbSommets; ++i)
        {
            x = -((float)nbSommets/2.0f);
            for(int j = 0; j < nbSommets; ++j)
            {
                points[indice] = x;
                points[indice+1] = y;

                z = qGray(image.pixel(i, j)) / 10;

                points[indice+2] = z;
                indice += 3;

                x += 1.0f;

            }
            y -= 1.0f;
        }

        m_drawn = true;
    }

    int k = 0;

    for(int cpt = 0; cpt < nbSommets-1; ++cpt)
    {
        int j = 0;
        k = cpt * nbSommets * 3;

        for(int i = 0; i < (nbSommets)*2*3; i += 3)
        {

            sommets[i] = points[k];
            sommets[i+1] = points[k+1];
            sommets[i+2] = points[k+2];

            if(points[k+2] > 12)
            {
                color[j*3] = 1.0f;
                color[j*3+1] = 1.0f;
                color[j*3+2] = 1.0f;
            }
            else if(points[k+2] < 2)
            {
                color[j*3] = 0.00f;
                color[j*3+1] = 0.50f;
                color[j*3+2] = 1.00f;
            }
            else if(points[k+2] < 5)
            {
                color[j*3] = 0.12f;
                color[j*3+1] = 0.62f;
                color[j*3+2] = 0.33f;
            }
            else
            {
                color[j*3] = 0.65f;
                color[j*3+1] = 0.40f;
                color[j*3+2] = 0.15f;
            }

            if(j%2 == 0)
                k += nbSommets*3;
            else
                k = (k - nbSommets*3) + 3;

            ++j;

        }

        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, sommets);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, color);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glDrawArrays(m_mode, 0, nbSommets*2);

        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);


    }

    GLfloat fogColor[] = {0.5f, 0.5f, 0.5f, 1};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_DENSITY, 0.35f);
    glFogf(GL_FOG_START, 10.0f);
    glFogf(GL_FOG_END, 20.0f);


    m_program->release();

    ++m_frame;
}

//! [5]
