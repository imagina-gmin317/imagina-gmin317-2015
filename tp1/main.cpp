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
#include <QCursor>
#include <QtGlobal>

#include <time.h>
#include <vector>

//! [1]
class TriangleWindow : public OpenGLWindow
{
public:
    TriangleWindow();

    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;
   // bool event(QEvent* event)Q_DECL_OVERRIDE;

private:

    GLuint loadShader(GLenum type, const char *source);

    GLfloat *getSurface(int nbPoint, GLfloat* points);
    GLfloat *generatePoint(int nbPoint);


    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;
    GLfloat *vertices;
    int direction =0; // direction de la camera (0:vers l'avant, -1:vers l'arrière)

    QCursor* curs;

    /* Coordonnées de la caméra */
    float posX = 0;
    float posY = 0;

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
    vertices = generatePoint(16);
    //this->curs = new QCursor(Qt::BlankCursor);
    this->posX=0;
    this->posY=0;

    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);


}
//! [4]

//! [5]
void TriangleWindow::render()
{

    //this->curs->setPos(this->position().x() + width() * 0.5f, this->position().y() + height() * 0.5f);

    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 16.0f/9.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
    matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);
    m_program->setUniformValue(m_matrixUniform, matrix);

   /*GLfloat vertices[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f,1.0f,
        1.5f,0.0f,
        1.5f,1.0f,
        1.5f,1.5f,
       1.0f,1.5f


    };*/
    GLfloat *vertices = getSurface(16,this->vertices);
    GLfloat colors[] = {
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f
    };

    glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_LINE_STRIP, 0, 16*16);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    m_program->release();

    ++m_frame;
}

/**
 * @brief TriangleWindow::getSurface
 * @return tableau de nbPoint*nbPoint points à 3 paramètres, représentant une surface
 * organise les points passés en paramètre pour la création des triangles de la surface
 */
GLfloat* TriangleWindow::getSurface(int nbPoint, GLfloat* points){
    GLfloat *vertices = points;
    //GLfloat *vert;//les points organisés
    std::vector<GLfloat> vert;
    bool b = true; // permet de changer le sens de parcours des points
    for(int i =0;i<nbPoint;i++){
        for(int j=0;j<nbPoint;j++){
            if(b){// de gauche à droite
                vert.push_back(vertices[i*nbPoint*3+j]);//x
                vert.push_back(vertices[i*nbPoint*3+j+1]);//y
                vert.push_back(vertices[i*nbPoint*3+j+2]);//z

                vert.push_back(vertices[(i+1)*nbPoint*3+j]);//x
                vert.push_back(vertices[(i+1)*nbPoint*3+j+1]);//y
                vert.push_back(vertices[(i+1)*nbPoint*3+j+2]);//z
            }else{// de droite à gauche
                vert.push_back(vertices[i*nbPoint*3+nbPoint-j]);//x
                vert.push_back(vertices[i*nbPoint*3+nbPoint-j+1]);//y
                vert.push_back(vertices[i*nbPoint*3+nbPoint-j+2]);//z

                vert.push_back(vertices[(i+1)*nbPoint*3+nbPoint-j]);//x
                vert.push_back(vertices[(i+1)*nbPoint*3+nbPoint-j+1]);//y
                vert.push_back(vertices[(i+1)*nbPoint*3+nbPoint-j+2]);//z
            }

        }
        if(b){
            b=false;
        }
    }
    GLfloat *tabVertices = new GLfloat[vert.size()];
    for(int i=0;i<vert.size();i++){
        tabVertices[i]=vert.at(i);
    }

    return tabVertices;
}

/**
 * @brief TriangleWindow::generatePoint
 * @param nbPoint
 * @return un tableau de nbPoint*nbPoint pour créer une surface (dans getSurface)
 */
GLfloat* TriangleWindow::generatePoint(int nbPoint){
    GLfloat *point = new GLfloat[nbPoint*nbPoint*3];
    float stepX = 1.0f / (float)(nbPoint-1);
    float stepY = 1.0f / (float)(nbPoint-1);
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    int cpt =0;
    for(int i=0;i<nbPoint;i++){
        x = 0.0f;
        for(int j=0;j<nbPoint;j++){
            z = rand() % 2;
            point[cpt++]=x;
            point[cpt++]=y;
            point[cpt++]=z;
            x+=stepX;
        }
        y+=stepY;
    }
    return point;
}


/*bool TriangleWindow::event(QEvent* event){
    QMouseEvent* mouse;
    QKeyEvent* key;

    switch(event->type()){
    case QEvent::KeyPress :
        key = static_cast<QKeyEvent*>(event);
        if(key->key() == Qt::Key_Up){
            //direction = 0;
            qDebug()<<"chouette";
        }else if(key->key()==Qt::Key_Down){
            direction = -1;
        }else if (key->key()== 'q'){
            qApp->exit;
        }
    }
    return true;

    OpenGLWindow::event(event);
}*/


//! [5]
