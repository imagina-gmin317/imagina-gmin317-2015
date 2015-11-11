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
#include <iostream>
#include <time.h>

using namespace std;

#define longueurCarre 240//16*16
GLfloat tabPts[longueurCarre*longueurCarre*3];
bool firstTime = true;

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
    firstTime = true;
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);

    TriangleWindow window;
    window.setFormat(format);
    window.resize(640, 480);
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
}
//! [4]

QColor getColor(float z)
{
    if(z < 10.0f/5.0f)
        return QColor(65,105,225,255);//Bleu foncé
    else if(z < 20.0f/5.0f)
        return QColor(0,191,255,255);//Bleue clair
    else if(z < 50.0f/5.0f)
        return QColor(46,139,87,255);//Vert
    else if(z < 130.0f/5.0f)
        return QColor(126, 88, 53,255);//Marron
    else if(z < 200.0f/5.0f)
        return QColor(220, 220, 220,255);//Gris clair
    else
        return QColor(255,255,255,255);//Blanc
}

//! [5]
void TriangleWindow::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 4.0f/3.0f/*16.0f/9.0f*/, 0.1f, 500.0f);
    matrix.translate(this->m_x, this->m_y, this->m_z);

    matrix.rotate(this->m_rotate_y, 1, 0, 0);
    matrix.rotate(this->m_rotate_x, 0, 0, 1);

    m_program->setUniformValue(m_matrixUniform, matrix);

    GLfloat x = -60.0f;
    GLfloat y = -60.0f;
    GLfloat z = 1.0f;
    GLfloat dist = 1.0;

    int cpt = 0;
    glEnable(GL_FOG);
    GLfloat fogColor[] = {0.5f, 0.5f, 0.5f, 1};
    //glClearColor(0.5f, 0.5f, 0.5f, 1);

    if(firstTime)
    {
        QImage img ("C:/Users/CloudDeLuna/Documents/GitHub/imagina-gmin317-2015/tp1/heightmap-1.png");
        if (!img.isNull())
        {
            srand(time(NULL));
            for (int i =0; i < longueurCarre; ++i)
            {
                x = 0.0f;
                for(int k = 0; k < longueurCarre; ++k)
                {
                    tabPts[cpt] = y;
                    tabPts[cpt+1] = x;
                    tabPts[cpt+2] = -(qGray(img.pixel(i, k)) / 5);

                    cpt += 3;
                    x += dist;
                }
                y += dist;
            }
        }
        else
        {
            srand(time(NULL));
            for (int i =0; i < longueurCarre; ++i)
            {
                x = 0.0f;
                for(int k = 0; k < longueurCarre; ++k)
                {
                    tabPts[cpt] = y;
                    tabPts[cpt+1] = x;
                    tabPts[cpt+2] = rand()%100 > 70 ? (float)rand()/RAND_MAX : 0.0f;//(float)rand()/RAND_MAX;

                    cpt += 3;
                    x += dist;
                }
                y += dist;
            }
        }
        firstTime = false;
    }

    int longCarrCube = longueurCarre*3;
    int j = 0;

    for(int i = 0; i < longueurCarre - 1; ++i)
    {
        GLfloat vertices[(longueurCarre-1)*3*3*2];
        GLfloat colors[(longueurCarre-1)*3*3*2];

        j = i* longCarrCube;

        for(int k = 0; k < (longueurCarre-1)*3*3*2; k += 18)
        {
            vertices[k]     = tabPts[j];
            vertices[k+1]   = tabPts[j+1];
            vertices[k+2]   = tabPts[j+2];

            QColor color = getColor(-tabPts[j+2]);
            colors[k]   = color.red()/255.0f;
            colors[k+1] = color.green()/255.0f;
            colors[k+2] = color.blue()/255.0f;

            vertices[k+3] = tabPts[j+3];
            vertices[k+4] = tabPts[j+4];
            vertices[k+5] = tabPts[j+5];

            color = getColor(-tabPts[j+5]);
            colors[k+3]  = color.red()/255.0f;
            colors[k+4] = color.green()/255.0f;
            colors[k+5] = color.blue()/255.0f;

            vertices[k+6] = tabPts[j+longCarrCube];
            vertices[k+7] = tabPts[j+longCarrCube+1];
            vertices[k+8] = tabPts[j+longCarrCube+2];

            color = getColor(-tabPts[j+longCarrCube+2]);
            colors[k+6]  = color.red()/255.0f;
            colors[k+7] = color.green()/255.0f;
            colors[k+8] = color.blue()/255.0f;

            vertices[k+9] = tabPts[j+3];
            vertices[k+10] = tabPts[j+4];
            vertices[k+11] = tabPts[j+5];

            color = getColor(-tabPts[j+5]);
            colors[k+9]  = color.red()/255.0f;
            colors[k+10] = color.green()/255.0f;
            colors[k+11] = color.blue()/255.0f;

            vertices[k+12] = tabPts[j+longCarrCube+3];
            vertices[k+13] = tabPts[j+longCarrCube+4];
            vertices[k+14] = tabPts[j+longCarrCube+5];

            color = getColor(-tabPts[j+longCarrCube+5]);
            colors[k+12]  = color.red()/255.0f;
            colors[k+13] = color.green()/255.0f;
            colors[k+14] = color.blue()/255.0f;

            vertices[k+15] = tabPts[j+longCarrCube];
            vertices[k+16] = tabPts[j+longCarrCube+1];
            vertices[k+17] = tabPts[j+longCarrCube+2];

            color = getColor(-tabPts[j+longCarrCube+2]);
            colors[k+15]  = color.red()/255.0f;
            colors[k+16] = color.green()/255.0f;
            colors[k+17] = color.blue()/255.0f;

            j += 3;
        }

        glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);

        //glEnable(GL_CULL_FACE);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(this->m_affichage, 0, (longueurCarre-1)*2*3);
        //GL_LINE_LOOP
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
    }

    m_program->release();

    ++m_frame;
}
//! [5]
