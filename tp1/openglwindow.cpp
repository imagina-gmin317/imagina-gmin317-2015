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

#include <QtCore/QCoreApplication>

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>
#include <QKeyEvent>

//! [1]
OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QWindow(parent)
    , m_update_pending(false)
    , m_animating(false)
    , m_context(0)
    , m_device(0)
    , m_x(0.0f) // DEPLACEMENT SUR L'AXE X
    , m_y(0.0f) // DEPLACEMENT SUR L'AXE Y
    , m_z(-350.0f) // DEPLACEMENT SUR L'AXE Z
    , m_drawn(false) // VARIABLE POUR NE PLUS RECHARGER LE TABLEAU DE POINT
    , m_mode(GL_LINE_LOOP) // MODE POUR CHOISIR SI ON AFFICHE LES TRIANGLES EN STRIP OU BIEN EN FIL DE FER
    , m_xrotation(0.0f) // VARIABLE POUR LA ROTATION
    , m_zrotation(0.0f)
    , m_axe(0) // VARIABLE POUR CHOISIR L'AXE DE ROTATION
    , m_xmouse(0)
    , m_ymouse(0)
    , m_mouseButtonPressed(false)
{
    setSurfaceType(QWindow::OpenGLSurface);
}
//! [1]

OpenGLWindow::~OpenGLWindow()
{
    delete m_device;
}
//! [2]
void OpenGLWindow::render(QPainter *painter)
{
    Q_UNUSED(painter);
}

void OpenGLWindow::initialize()
{
}

void OpenGLWindow::render()
{
    if (!m_device)
        m_device = new QOpenGLPaintDevice;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_device->setSize(size());

    QPainter painter(m_device);
    render(&painter);
}
//! [2]

//! [3]
void OpenGLWindow::renderLater()
{
    if (!m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

bool OpenGLWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        m_update_pending = false;
        renderNow();
        return true;
    case QEvent::MouseMove:
    {
        QMouseEvent *MouseEvent = (QMouseEvent*)event;

        if(!m_mouseButtonPressed)
        {
            m_xmouse = MouseEvent->x();
            m_ymouse = MouseEvent->y();
        }

        if(MouseEvent->buttons() & Qt::LeftButton)
        {
            m_mouseButtonPressed = true;

            if(MouseEvent->x() > m_xmouse)
                m_zrotation += 5.0f;
            else
                m_zrotation -= 5.0f;

            m_axe = 0;

            m_xmouse = MouseEvent->x();

            renderNow();
        }
        else if(MouseEvent->buttons() & Qt::RightButton)
        {
            m_mouseButtonPressed = true;

            if(MouseEvent->y() > m_ymouse)
                m_xrotation += 5.0f;
            else
                m_xrotation -= 5.0f;

            m_axe = 1;

            m_ymouse = MouseEvent->y();

            renderNow();
        }
        else
        {
            m_mouseButtonPressed = false;
        }
        return true;
    }
    case QEvent::Wheel:
    {
        QWheelEvent *WheelEvent = (QWheelEvent*)event;
        if(WheelEvent->delta() > 0)
        {
            m_z += 10.0f;
        }
        else
        {
            m_z -= 10.0f;
        }
        renderNow();
        return true;
    }
    case QEvent::KeyPress:
    {
        QKeyEvent *KeyEvent = (QKeyEvent*)event;
        int key = KeyEvent->key();

        switch(key) {
            case Qt::Key_E :
                m_y += 5.0f;
                break;
            case Qt::Key_S :
                m_x -= 5.0f;
                break;
            case Qt::Key_D :
                m_y -= 5.0f;
                break;
            case Qt::Key_F :
                m_x += 5.0f;
                break;
            case Qt::Key_Up :
                m_z += 2.0f;
                break;
            case Qt::Key_Down :
                m_z -= 2.0f;
                break;
            case Qt::Key_1 :
                m_mode = GL_TRIANGLE_STRIP;
                break;
            case Qt::Key_2 :
                m_mode = GL_LINE_LOOP;
                break;

            default:
                return true;
        }

        renderNow();
        return true;
    }
    default:
        return QWindow::event(event);
    }
}

void OpenGLWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}
//! [3]

//! [4]
void OpenGLWindow::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);

    if (m_animating)
        renderLater();
}
//! [4]

//! [5]
void OpenGLWindow::setAnimating(bool animating)
{
    m_animating = animating;

    if (animating)
        renderLater();
}
//! [5]

