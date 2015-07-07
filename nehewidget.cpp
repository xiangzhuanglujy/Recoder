#include "nehewidget.h"

#include <GL/glu.h>
#include <QImage>
#include <math.h>
#include <QFont>
#include <QDebug>
#include <QPainter>

GLfloat lightAmbient[4] = { 0.5, 0.5, 0.5, 1.0 };
GLfloat lightDiffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat lightPosition[4] = { 0.0, 0.0, 2.0, 1.0 };
GLuint	base=0;				// Base Display List For The Font Set

GLfloat	cnt1=0.0;				// 1st Counter Used To Move Text & For Coloring
GLfloat	cnt2=0.0;				// 1st Counter Used To Move Text & For Coloring


GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
    char		text[256];								// Holds Our String
    va_list		ap;										// Pointer To List Of Arguments

    if (fmt == NULL)									// If There's No Text
        return;											// Do Nothing

    va_start(ap, fmt);									// Parses The String For Variables
    vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
    va_end(ap);											// Results Are Stored In Text

    glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
    glListBase(base - 32);								// Sets The Base Character to 32
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
    glPopAttrib();										// Pops The Display List Bits
}

NeHeWidget::NeHeWidget(QWidget *parent)
    :QGLWidget(parent)
{
    xRot=yRot=zRot=0.0;
    zoom=-15.0;
    xSpeed=ySpeed=0.0;

    filter=0;
    light=false;
    fullscreen=false;

    rTri=0.0;
    rQuard=0.0;
    bLend=false;

    tilt = 90.0;
    spin = 0.0;
    loop = 0;
    twinkle = false;

    startTimer(5);
}

NeHeWidget::~NeHeWidget()
{
//    glDeleteLists(base, 96);							// Delete All 96 Characters
}

void NeHeWidget::loadGLTextures()
{
    QImage tex,buf;
    if ( !buf.load( ".\\Star.bmp" ) )
    {
        qWarning( "Could not read image file, using single-color instead." );
        QImage dummy(128,128,QImage::Format_RGB32);
        dummy.fill(Qt::green);
        buf = dummy;
    }
    tex = QGLWidget::convertToGLFormat( buf );

    glGenTextures( 1, &texture[0] );
    glBindTexture( GL_TEXTURE_2D, texture[0] );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex.width(), tex.height(), 0,
      GL_RGBA, GL_UNSIGNED_BYTE, tex.bits() );
}

void NeHeWidget::timerEvent(QTimerEvent *)
{
    updateGL();
}

void NeHeWidget::buildFonts()
{
//    QPainter paint(this);
//    QFont font;
//    font.setPointSize(50);
//    font.setUnderline(true);
//    paint.setFont(font);
    base = glGenLists(96);								// Storage For 96 Characters
//    glBindTexture(GL_TEXTURE_2D, texture[0]);			// Select Our Font Texture

    qDebug()<<wglUseFontBitmaps(wglGetCurrentDC(),32,96,base)<<base;
}

void NeHeWidget::initializeGL()
{
    loadGLTextures();

    glEnable( GL_TEXTURE_2D );
    glShadeModel( GL_SMOOTH );
    glClearColor( 0.0, 0.0, 0.0, 0.5 );
    glClearDepth( 1.0 );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    glEnable( GL_BLEND );

    for ( loop = 0; loop < num; loop++ )
    {
        star[loop].angle = 0.0;
        star[loop].dist = ( float(loop)/num ) * 5.0;
        star[loop].r = rand() % 256;
        star[loop].g = rand() % 256;
        star[loop].b = rand() % 256;
    }

//    buildFonts();
}

void NeHeWidget::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glBindTexture( GL_TEXTURE_2D, texture[0] );

    for ( loop = 0; loop < num; loop++ )
    {
        //    这段程序我们来循环绘制所有的星星。
        glLoadIdentity();
        //    绘制每颗星星之前，重置模型观察矩阵。
        glTranslatef( 0.0, 0.0, zoom );
        //    深入屏幕里面（使用“zoom”的值）。
        glRotatef( tilt, 1.0, 0.0, 0.0 );
        //    倾斜视角（使用“tilt”的值）。
        glRotatef( star[loop].angle, 0.0, 1.0, 0.0 );
        glTranslatef( star[loop].dist, 0.0, 0.0 );
        glRotatef( -star[loop].angle, 0.0, 1.0, 0.0 );
        glRotatef( -tilt, 1.0, 0.0, 0.0 );

        if ( twinkle )
        {
            glColor4ub( star[(num-loop)-1].r,
            star[(num-loop)-1].g,
            star[(num-loop)-1].b, 255 );
            glBegin( GL_QUADS );
            glTexCoord2f( 0.0, 0.0 ); glVertex3f( -1.0, -1.0, 0.0 );
            glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, -1.0, 0.0 );
            glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
            glTexCoord2f( 0.0, 1.0 ); glVertex3f( -1.0, 1.0, 0.0 );
            glEnd();
        }

        glRotatef( spin, 0.0, 0.0, 1.0 );
        glColor4ub( star[loop].r, star[loop].g, star[loop].b, 255 );
        glBegin( GL_QUADS );
        glTexCoord2f( 0.0, 0.0 ); glVertex3f( -1.0, -1.0, 0.0 );
        glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, -1.0, 0.0 );
        glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
        glTexCoord2f( 0.0, 1.0 ); glVertex3f( -1.0, 1.0, 0.0 );
        glEnd();

        spin += 0.01;
        star[loop].angle += float(loop)/num;
        star[loop].dist -= 0.01;

        if ( star[loop].dist < 0.0 )
        {
            star[loop].dist += 5.0;
            star[loop].r = rand() % 256;
            star[loop].g = rand() % 256;
            star[loop].b = rand() % 256;
        }
    }

//    glLoadIdentity();									// Reset The Current Modelview Matrix
//    glTranslatef(0.0f,0.0f,-1.0f);						// Move One Unit Into The Screen
//    glColor3f(1.0f*float(cos(cnt1)),1.0f*float(sin(cnt2)),1.0f-0.5f*float(cos(cnt1+cnt2)));
//    // Position The Text On The Screen
//    glRasterPos2f(-0.45f+0.05f*float(cos(cnt1)), 0.32f*float(sin(cnt2)));

//    glPrint("Active OpenGL Text With NeHe", cnt1);	// Print GL Text To The Screen
//    cnt1+=0.051f;										// Increase The First Counter
//    cnt2+=0.005f;										// Increase The First Counter
}

void NeHeWidget::resizeGL(int width, int height)
{
    if ( height == 0 )
    {
        height = 1;
    }
    glViewport( 0, 0, (GLint)width, (GLint)height );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 45.0, (GLfloat)width/(GLfloat)height, 0.1, 100.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}
