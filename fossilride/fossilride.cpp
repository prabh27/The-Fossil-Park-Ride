#include<GL/freeglut.h>
#include <iostream>
#include <stdlib.h>
#include<math.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "imageloader.h"
#include "vec3f.h"
#include<stdlib.h>
#include<cstring>
#include<cstdio>
#include<vector>
#include<time.h>
#define ESC 27
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define RAD2DEG(rad) (rad * 180 / PI)


/*#include <iostream>
#include <stdlib.h>
#include<math.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <GL/freeglut.h>
#include "imageloader.h"
#include "vec3f.h"
#include<stdlib.h>
#include<stdio.h>
#include<vector>
#include<time.h>
#include<string.h>
#define ESC 27
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define RAD2DEG(rad) (rad * 180 / PI)
*/
//void gameover_scene();
void drawBox(float len,float width);

using namespace std;
void drawbike();
void Set_Camera();
//Represents a terrain, by storing a set of heights and normals at 2D locations
class Terrain {
    private:
        int w; //Width
        int l; //Length
        float** hs; //Heights
        Vec3f** normals;
        bool computedNormals; //Whether normals is up-to-date
    public:
        Terrain(int w2, int l2) {
            w = w2;
            l = l2;

            hs = new float*[l];
            for(int i = 0; i < l; i++) {
                hs[i] = new float[w];
            }

            normals = new Vec3f*[l];
            for(int i = 0; i < l; i++) {
                normals[i] = new Vec3f[w];
            }

            computedNormals = false;
        }

        ~Terrain() {
            for(int i = 0; i < l; i++) {
                delete[] hs[i];
            }
            delete[] hs;

            for(int i = 0; i < l; i++) {
                delete[] normals[i];
            }
            delete[] normals;
        }

        int width() {
            return w;
        }

        int length() {
            return l;
        }

        //Sets the height at (x, z) to y
        void setHeight(int x, int z, float y) {
            hs[z][x] = y;
            computedNormals = false;
        }

        //Returns the height at (x, z)
        float getHeight(int x, int z) {
            if(z>=0 && x>=0)
                return hs[z][x];
            else
                return 0;
        }

        //Computes the normals, if they haven't been computed yet
        void computeNormals() {
            if (computedNormals) {
                return;
            }

            //Compute the rough version of the normals
            Vec3f** normals2 = new Vec3f*[l];
            for(int i = 0; i < l; i++) {
                normals2[i] = new Vec3f[w];
            }

            for(int z = 0; z < l; z++) {
                for(int x = 0; x < w; x++) {
                    Vec3f sum(0.0f, 0.0f, 0.0f);

                    Vec3f out;
                    if (z > 0) {
                        out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
                    }
                    Vec3f in;
                    if (z < l - 1) {
                        in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
                    }
                    Vec3f left;
                    if (x > 0) {
                        left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
                    }
                    Vec3f right;
                    if (x < w - 1) {
                        right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
                    }

                    if (x > 0 && z > 0) {
                        sum += out.cross(left).normalize();
                    }
                    if (x > 0 && z < l - 1) {
                        sum += left.cross(in).normalize();
                    }
                    if (x < w - 1 && z < l - 1) {
                        sum += in.cross(right).normalize();
                    }
                    if (x < w - 1 && z > 0) {
                        sum += right.cross(out).normalize();
                    }

                    normals2[z][x] = sum;
                }
            }

            //Smooth out the normals
            const float FALLOUT_RATIO = 0.5f;
            for(int z = 0; z < l; z++) {
                for(int x = 0; x < w; x++) {
                    Vec3f sum = normals2[z][x];

                    if (x > 0) {
                        sum += normals2[z][x - 1] * FALLOUT_RATIO;
                    }
                    if (x < w - 1) {
                        sum += normals2[z][x + 1] * FALLOUT_RATIO;
                    }
                    if (z > 0) {
                        sum += normals2[z - 1][x] * FALLOUT_RATIO;
                    }
                    if (z < l - 1) {
                        sum += normals2[z + 1][x] * FALLOUT_RATIO;
                    }

                    if (sum.magnitude() == 0) {
                        sum = Vec3f(0.0f, 1.0f, 0.0f);
                    }
                    normals[z][x] = sum;
                }
            }

            for(int i = 0; i < l; i++) {
                delete[] normals2[i];
            }
            delete[] normals2;

            computedNormals = true;
        }

        //Returns the normal at (x, z)
        Vec3f getNormal(int x, int z) {
            if (!computedNormals) {
                computeNormals();
            }
            return normals[z][x];
        }
};

//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
Terrain* loadTerrain(const char* filename, float height) {
    Image* image = loadBMP(filename);
    int  span=2;
    Terrain* t = new Terrain(image->width*span, image->height*span);
    for(int y = 0; y < image->height*span; y++) {
        for(int x = 0; x < image->width*span; x++) {
            unsigned char color =
                (unsigned char)image->pixels[3 * ((y%image->height) * image->width + x%image->width)];
            float h = height * ((color / 255.0f) - 0.5f);
            if(h<0)
                t->setHeight(x, y, 0);
            else
                t->setHeight(x, y, h);

        }
    }
    delete image;
    t->computeNormals();
    return t;
}

void drawBox(float len, float width) {

    glLineWidth(3.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_QUADS);
    glVertex2f(-len / 2, -width / 2);
    glVertex2f(len / 2, -width / 2);
    glVertex2f(len / 2, width / 2);
    glVertex2f(-len / 2, width / 2);
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

float _angle = 60.0f;
Terrain* _terrain;

void cleanup() {
    delete _terrain;
}

GLuint loadTexture(Image* image) {
    GLuint textureId;
    glGenTextures(1, &textureId); //Make room for our texture
    glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
    //Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
            0,                            //0 for now
            GL_RGB,                       //Format OpenGL uses for image
            image->width, image->height,  //Width and height
            0,                            //The border of the image
            GL_RGB, //GL_RGB, because pixels are stored in RGB format
            GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored                                                                                       //as unsigned numbers
            image->pixels);               //The actual pixel data
    return textureId; //Returns the id of the texture
}

GLuint _textureId; //The id of the texture

void initRendering() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    Image* image = loadBMP("vtr.bmp");
    _textureId = loadTexture(image);
    delete image;
}
//Returns the approximate height of the terrain at the specified (x, z) position
float heightAt(Terrain* terrain, float x, float z) {
    //Make (x, z) lie within the bounds of the terrain
    if (x < 0) {
        x = 0;
    }
    else if (x > terrain->width() - 1) {
        x = terrain->width() - 1;
    }
    if (z < 0) {
        z = 0;
    }
    else if (z > terrain->length() - 1) {
        z = terrain->length() - 1;
    }

    //Compute the grid cell in which (x, z) lies and how close we are to the
    //left and outward edges
    int leftX = (int)x;
    if (leftX == terrain->width() - 1) {
        leftX--;
    }
    float fracX = x - leftX;

    int outZ = (int)z;
    if (outZ == terrain->width() - 1) {
        outZ--;
    }
    float fracZ = z - outZ;

    //Compute the four heights for the grid cell
    float h11 = terrain->getHeight(leftX, outZ);
    float h12 = terrain->getHeight(leftX, outZ + 1);
    float h21 = terrain->getHeight(leftX + 1, outZ);
    float h22 = terrain->getHeight(leftX + 1, outZ + 1);

    //Take a weighted average of the four heights
    return (1 - fracX) * ((1 - fracZ) * h11 + fracZ * h12) +
        fracX * ((1 - fracZ) * h21 + fracZ * h22);
}

//Global variables
float box_len = 7.0f;
float box_width = 4.0f;
float height_previous=-1;
bool applied_brake=0;
float fossil_angle = 0.0f;
float prevpitch=0;
int _time=60;
float acceleration=0,retard=0;
float velocity=0;
float yaw=0; //angle about y to tell where it is going
float pitch=0; //angle to increase hight
float roll=0;//tilt
bool flag_jump=0;
int cammode=0;
float bike_x=0,bike_z=0,bike_y=0.375;
float blook_x=0.0,blook_y=1.0,blook_z=0.0;
float x = 0.0, y = -5.0; // initially 5 units south of origin
float deltaMove = 0.0; // initially camera doesn't move
int ifrolled;
int fossil_count=0;
// Camera direction
float lx = 0.0, ly = 1.0; // camera points initially along y-axis
float angle = 0.0; // angle of rotation for the camera direction
float deltaAngle = 0.0; // additional angle change when dragging
float deltaRotate=0;
float avg_h,h1,h2;//,h2,h3,h4,h5,h6,h7,h8,h9;
//vector < pair<int,int> >fossil;
float fossil_size=0.5f;
float tyre_angle=0;
time_t previous_time=time(0);
time_t current_time;
int gameover=0;
int pause = 0;
int restart=0;
int song_flag=1;
bool lightflag=false;
struct Fossil {
    float x;
    float z;
    int visible;
};

struct fossil_list
{
        Fossil* fossil;
        struct fossil_list *next;
};
fossil_list *head;
void insert_end(Fossil *x)
{
    struct fossil_list *ptr;
    ptr=head;
    if(ptr==NULL)
    {
        ptr=(fossil_list *)malloc(sizeof(struct fossil_list));
        ptr->fossil=x;
        ptr->next=NULL;
        head=ptr;
    }
    else
    {
        while(ptr->next!=NULL)
            ptr=ptr->next;
        ptr->next=(fossil_list *)malloc(sizeof(struct fossil_list));
        ptr->next->fossil=x;
        ptr->next->next=NULL;
    }
}


void handleResize(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}

float getmagnitude(float a,float b,float c)
{
    return sqrt((a*a)+(b*b)+(c*c));
}

void coloringset()
{
    GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

    GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

}

void printtext(float x, float y, string String)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glRasterPos2f(x,y);
    for (size_t i=0; i<String.size(); i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, String[i]);
    }
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void drawScene() {
      // glRasterPos3f(0.0f,0.0f,0.0f);
   /* for(int l=0;diff[l]!='\0';l++)
        glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, diff[l]);
    char text2[15];
    glColor3f(0.5f,0.5f,0.5f);
    sprintf(text2, "Score : %d",fossil_count);
    glRasterPos3f(3.0f - 1.0f,2.0f-0.5f,0.0f);
    for(int l=0;text2[l]!='\0';l++)
        glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, text2[l]);
        */
    song_flag-=1;
  
    if(gameover==0)
    {
        if(song_flag==0)
        {
            system("aplay music.WAV &");
            song_flag=1000;
        }
        if(!lightflag)
            glClearColor(0.0,0.0,0.0,1.0);
        else
            glClearColor(0.0,0.7,1.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        char diff[15];
        glColor3f(1.0f,1.0f,0.0f);
        sprintf(diff, "The Fossil park ride");
        printtext(-0.2,0.9,string(diff));
        glColor3f(1.0f,1.0f,0.0f);
        sprintf(diff, "Score: %d",fossil_count);
        printtext(0.6,0.8,string(diff)); 
        sprintf(diff, "Time left: %d",_time);
        printtext(0.6,0.9,string(diff));
        if(head==NULL)
        {
            sprintf(diff,"Game Over\n");
            printtext(-0.04f,0.0f,string(diff));
            sprintf(diff,"Score: %d\n",fossil_count);
            printtext(-0.04f,-0.1f,string(diff));
            sprintf(diff,"Time left: %d\n",_time);
            printtext(-0.04f,-0.2f,string(diff));
            glutSwapBuffers();
            return;
        }
        else if(_time<=0)
        {
            gameover=1;
            sprintf(diff,"Game Over\n");
            printtext(-0.04f,0.0f,string(diff));
            sprintf(diff,"Score: %d\n",fossil_count);
            printtext(-0.04f,-0.1f,string(diff));
            sprintf(diff,"Time left: %d\n",_time);
            printtext(-0.04f,-0.2f,string(diff));
            gameover=1;
            glutSwapBuffers();
            return;
        }
        glTranslatef(0.0f, 0.0f, 0.0f);
        if(applied_brake)
        {
            velocity=0;
            retard=0;
            acceleration=0;
        }
        if(velocity>=0 || pitch!=0) 
        {
            bike_z+=velocity*cosf(DEG2RAD(yaw));
            bike_x+=velocity*sinf(DEG2RAD(yaw));
        }
        else if(velocity<0)
        {
            velocity=0;
            acceleration=0;
            retard=0;
        }
        if(bike_z<0)
            bike_z=0;
        if(bike_x<0)
            bike_x=0;
        if(bike_z>117)
            bike_z=117;
        if(bike_x>117)
            bike_x=117;
        h1=heightAt(_terrain,bike_x,bike_z);
        h2=h1;
        if(flag_jump==1)
            h1=height_previous-0.1;
        if(height_previous-(h2)>0.2)
            flag_jump=1;
        else if(h1<0)
            flag_jump=0;
        height_previous=h1;
        Vec3f tnormal = _terrain->getNormal(bike_x, bike_z);
        if(!flag_jump)
        {
            float variable,x,y,z;
            x=sinf(DEG2RAD(yaw));
            y=0;
            z=cosf(DEG2RAD(yaw));
            variable=((tnormal[0]*x+tnormal[2]*z+tnormal[1]*y)/(getmagnitude(tnormal[0],tnormal[1],tnormal[2])*getmagnitude(x,y,z)));
            variable=RAD2DEG(acos(variable));
            pitch=-(90-variable);
        }
        bike_y=h1+0.25;
        if(cammode%5==0)
        {
            // Driver View
            gluLookAt(
                    bike_x-(2*sin(DEG2RAD(yaw))), bike_y+1,bike_z-(2*cos(DEG2RAD(yaw))),
                    bike_x,      bike_y+1,      bike_z,
                    0.0,    1.0,    0.0
                    );

        }
        else if(cammode%5==1)
        {
            // Wheel View
            gluLookAt(
                    bike_x,      bike_y+0.25,      bike_z,
                    bike_x+2*sinf(DEG2RAD(yaw)), bike_y+0.25,bike_z+2*cosf(DEG2RAD(yaw)),
                    0.0,    1.0,    0.0
                    );

        }
        else if(cammode%5==2)
        {
            //OverHead View
            gluLookAt(
                    bike_x,      bike_y+10,      bike_z,
                    bike_x+sin(DEG2RAD(yaw)), bike_y,bike_z+cos(DEG2RAD(yaw)),
                    0.0,    1.0,    0.0
                    );
        }
        else if(cammode%5==3)
        {

            // Hellicopter View
            gluLookAt(
                    bike_x-3,     bike_y,bike_z,
                    bike_x, bike_y+0.25, bike_z,
                    0.0,    1.0,    0.0
                    );
        }
        else if(cammode%5==4)
        {
            // Follow Cam
            gluLookAt(
                    bike_x-(3.5*sin(DEG2RAD(yaw))), bike_y+1,bike_z-(3.5*cos(DEG2RAD(yaw))),
                    bike_x,      bike_y+1,      bike_z,
                    0.0,    1.0,    0.0
                    );

        }
        GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

        GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
        GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
        glColor3f(1.0f, 0.6f, 0.2f);
        glEnable(GL_TEXTURE_2D);


        //Bottom
             for(int z = 0; z < _terrain->length() - 1; z++) {
            
            glBindTexture(GL_TEXTURE_2D, _textureId);
           glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         /*  float x1[]={0.0,0.0};
        float y1[]={0.0,1.0};
        float x2[]={1.0,1.0};
        float y2[]={0.0,1.0};*/

            //Makes OpenGL draw a triangle at every three consecutive vertices
            glBegin(GL_TRIANGLE_STRIP);
            for(int x = 0; x < _terrain->width(); x++) {
                float fet = (x%5)*1.0f;
                Vec3f normal = _terrain->getNormal(x, z);
                glNormal3f(normal[0], normal[1], normal[2]);
                glTexCoord2f(0.0f+fet,0.0f+fet);
                glVertex3f(x, _terrain->getHeight(x, z), z);
                normal = _terrain->getNormal(x, z + 1);
                glTexCoord2f(10.0f+fet,10.0f+fet);
                //glTexCoord2f(x2[(x+1)%2],y2[(x+1)%2]);
                glNormal3f(normal[0], normal[1], normal[2]);
                glTexCoord2f(0.0f+fet,20.0f+fet);
                //glTexCoord2f(x2[(x+1)%2],y2[(x+1)%2]);
                glVertex3f(x, _terrain->getHeight(x, z + 1), z + 1);
            }
            glEnd();
        } 
        glDisable(GL_TEXTURE_2D);
        drawbike();
        glColor3f(0.0f,0.0f,0.8f);
        glBegin(GL_QUADS); 
        glVertex3f(33.0f,0.2f,3.0f); 
        glVertex3f(117.0f,0.2f,43.0f); 
        glVertex3f(51.0f,0.2f,0.6f); 
        glVertex3f(117.0f,0.2f,31.0f); 
        glEnd();
        fossil_list *var = head;
        while(var!=NULL)
        {
            Fossil *temp_fossil = var->fossil;
            glPushMatrix();
            glTranslatef(temp_fossil->x,heightAt(_terrain,temp_fossil->x,temp_fossil->z), temp_fossil->z);
            glColor3f(1.0f, 1.0f, 0.0f);
            glRotatef(-90,1.0,0.0,0.0);
            glutSolidCylinder(0.75,2,20,20);
            glPopMatrix();
            var=var->next;
        }
        glutSwapBuffers();
    }
 /*   else
    {
        printf("111111111\n");
        gameover_scene();
    }*/
}
void update() 
{
    if(gameover==0)
    {
        if(pause==0)
        {
            fossil_angle += 2.0f;
            fossil_list *temp_fossil_list = head;
            fossil_list *fossil_previous = NULL;
            while(temp_fossil_list!=NULL)
            {
                Fossil *temp_fossil = temp_fossil_list->fossil;
                if((bike_x < temp_fossil->x + 0.75) && (bike_x > temp_fossil->x - 0.75))
                {
                    if((bike_z < temp_fossil->z + 0.75) && (bike_z > temp_fossil->z - 0.75))
                    {
                        fossil_count+=1;
                        if(fossil_previous == NULL)
                        {
                            head = temp_fossil_list->next;
                            free(temp_fossil_list);
                            temp_fossil_list = (fossil_list *)malloc(sizeof(fossil_list));
                            temp_fossil_list = head;
                        }
                        else if(fossil_previous!=NULL)
                        {
                            fossil_previous->next=temp_fossil_list->next;
                            free(temp_fossil_list);
                            temp_fossil_list = (fossil_list *)malloc(sizeof(fossil_list));
                            temp_fossil_list = fossil_previous->next;
                        }
                    }
                    else
                    {
                        fossil_previous = temp_fossil_list;
                        temp_fossil_list = temp_fossil_list->next;
                    }
                }
                else
                {
                    fossil_previous = temp_fossil_list;
                    temp_fossil_list = temp_fossil_list->next;
                }
            }
            if(flag_jump)
            {
                glutPostRedisplay(); // redisplay everything
                return;
            }
            if(prevpitch<0 && pitch==0)
            {
                retard=-0.005;
            }
            prevpitch=pitch;
            if(velocity>0)
                velocity=velocity+acceleration+retard-(0.005*sinf(DEG2RAD(pitch)));
            else 
            {
                retard=0;
                velocity=velocity+acceleration-(0.005*sinf(DEG2RAD(pitch)));
            }
            if(deltaRotate==-1)
                yaw+=5;
            else if(deltaRotate==1)
                yaw-=5;
            if( -45< (roll+(5*ifrolled)) && roll+(5*ifrolled)<45)
                roll=roll+(5*ifrolled);
            if(ifrolled)
                if(velocity!=0 && velocity>0.08)
                {
                    yaw+=-0.8*tanf(DEG2RAD(roll))/velocity;

                }
            time(&current_time);
            if(difftime(current_time,previous_time)>1)
            {
                previous_time=current_time;
                _time--;
            }

            glutPostRedisplay(); // redisplay everything
        }
    }
}


void drawbike()
{
    glColor3f(0.0,0.0,0.8); 
    glPushMatrix();
    glTranslatef(bike_x,0,bike_z);
    glRotatef(yaw,0,1,0);
    glRotatef(-pitch,1,0,0);
    glRotatef(roll,0,0,1);
    glTranslatef(0,bike_y+0.05,0);//to keep it till ground
    glPushMatrix();
    glTranslatef(0.0,-0.1,-0.33);
    glLineWidth(6);
    glBegin(GL_LINES);
    glColor3f(0.8,0.0,0.0); 
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(0.0f,0.25f,0.2f);
    glEnd();
    glColor3f(0.0,0.0,0.8); 
    glPushMatrix();
    glTranslatef(0.0,0.25f,0.1f);
    glutSolidCube(0.1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0,0.25f,0.2f);
    glutSolidCube(0.1);
    glPopMatrix();
    glPushMatrix();
        glColor3f(0.0,1.0,0.0);
        glTranslatef(0.0,0.35,0.3f);
        glutSolidCube(0.1);
    glPopMatrix();
    glPushMatrix();
        glColor3f(0.0,1.0,0.0);
        glTranslatef(0.0,0.45,0.3f);
        glutSolidCube(0.1);
    glPopMatrix();
    glPushMatrix();
        glColor3f(1.0,1.0,0.0);
        glTranslatef(0.0,0.55,0.3f);
        glutSolidSphere(0.1,20,20);
    glPopMatrix();
    glPushMatrix();
        glColor3f(0.0,1.0,1.0);
        glLineWidth(10);
        glBegin(GL_LINES);
        glVertex3f(-0.05f,0.43f,0.3f);
        glVertex3f(-0.100f,0.225f,0.550f);
        glEnd();
    glPopMatrix();
    glPushMatrix();
        glColor3f(0.0,1.0,1.0);
        glLineWidth(10);
        glBegin(GL_LINES);
        glVertex3f(0.05f,0.43f,0.3f);
        glVertex3f(0.100f,0.225f,0.550f);
        glEnd();
    glPopMatrix();
    glColor3f(0.0,0.0,0.8); 
    glPushMatrix();
    glTranslatef(0.0,0.25f,0.3f);
    glutSolidCube(0.1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0,0.25f,0.4f);
    glutSolidCube(0.1);
    glPopMatrix();
    glPushMatrix();
        glTranslatef(0.0,0.25f,0.45f);
        glLineWidth(4);
        glBegin(GL_LINES);
        glVertex3f(0.0f,0.0f,0.0f);
        glVertex3f(0.0f,0.0f,0.2f);
    glEnd();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.0,0.25f,0.5f);
        glLineWidth(6);
        glBegin(GL_LINES);
        glColor3f(1.0,0.0,0.0); 
        glVertex3f(0.0f,0.0f,0.0f);
        glVertex3f(0.0f,-0.25f,0.2f);
    glEnd();
    glPopMatrix();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0,0.0,0.125);
    glTranslatef(0.0,0.0,0.125);
    
    //headlight
    glPushMatrix();
    glColor3f(1.0,0.501,0.0); 
    glTranslatef(0.0,0.175,0.0);
    GLfloat light1_ambient[] = { 1, 1, 1, 1.0 };
    GLfloat light1_diffuse[] = { 1.0, 1.0, 1.0, 1.0 }; 
    GLfloat light1_specular[] = { 1.0, 1.0, 1.0, 1.0 }; 
    GLfloat light1_position[] = { -0.2, 0.0, 0.0, 1.0 };
    GLfloat spot_direction[] = { 0, 0.0, 1 };
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse); 
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.5);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.2);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.1); 
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 25.0);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0); 
    if(!lightflag) 
    {
        glEnable(GL_LIGHT1); 
    }
    else 
    { 
        glDisable(GL_LIGHT1); 
    }
    glutSolidCone(0.05,0.1,20,20);
    glPopMatrix();

    //handle
    glPushMatrix();
    glColor3f(0.25,0.25,0.25); 
    glLineWidth(5);
    glBegin(GL_LINES);
    glVertex3f(0.125,0.125,0.0);
    glVertex3f(0.0,0.125,0.0);
    glVertex3f(-0.125,0.125,0.0);
    glVertex3f(0.0,0.125,0.0);
    glEnd();
    glPopMatrix();


    //mirrors
    glPushMatrix();
    glColor3f(0.4,1.0,0.4); 
    glLineWidth(5);
    glBegin(GL_LINES);
    glVertex3f(0.125,0.125,0.0);
    glVertex3f(0.375,0.375,0.0);
    glVertex3f(-0.125,0.125,0.0);
    glVertex3f(-0.375,0.375,0.0);
    glEnd();
    glPopMatrix();
    
    glPopMatrix();


    tyre_angle+=velocity/0.06;
   
    //back wheel
    glPushMatrix();
    glColor3f(0.0,0.0,0.0); 
    glTranslatef(0.0,-0.125,-0.375);
    glRotatef(-90,1,0,0);
    glRotatef(90,0,1,0);
    glRotatef(RAD2DEG(tyre_angle),0,0,1);//wheel rotation
    glutWireTorus(0.06,0.12,10,10);
    glPopMatrix();

    //front wheel
    glPushMatrix();
    glColor3f(0.0,0.0,0.0);
    glTranslatef(0.0,-0.125,0.375);
    glRotatef(-90,1,0,0);
    glRotatef(90,0,1,0);
    glRotatef(RAD2DEG(tyre_angle),0,0,1);//wheel rotation
    glutWireTorus(0.06,0.12,10,10);
    glPopMatrix();

    glPopMatrix();
  }
float  view=1;
void processNormalKeys(unsigned char key, int xx, int yy)
{
    if (key == ESC || key == 'q' || key == 'Q')
    {
            cleanup();
            exit(0);
    }
    if(key==49)
        cammode = 0;
    else if(key==50)
        cammode = 1;
    else if(key==51)
        cammode = 2;
    else if(key==52)
        cammode = 3;
    else if(key==53)
        cammode = 4;
    else if(key == 'p')
    {
        if(pause==0)
        {
            pause=1;
        }
        else if(pause==1)
        {
            pause=0;
        }
    }
    else if(key == 'z')
    {
        restart=1;
    }
    if(!flag_jump && key=='a')
    {
        deltaRotate=-1;
        yaw-=5;
    }
    else if(!flag_jump && key=='d')
    {
        deltaRotate = 1.0; 
        yaw+=5;
    }
    if(key=='l')
    {
        if(lightflag==true)
        {
            lightflag=false;
        }
        else
        {
            lightflag=true;
        }
    }
} 
void KeyUp(unsigned char key, int x, int y) 
{
    switch (key) {
        case 'a' :
            deltaRotate = 0.0; 
            break;
        case 'd':
            deltaRotate = 0.0; 
            break;
    }
}  
void pressSpecialKey(int key, int xx, int yy)
{
    switch (key) {
        case GLUT_KEY_UP : 
            deltaMove = 1.0;
            acceleration=0.005;
            break;
        case GLUT_KEY_DOWN : 
            deltaMove = -1.0;
            applied_brake=1;
            // ret=-1; 
            break;
        case GLUT_KEY_RIGHT :
            ifrolled=1;
            break;
        case GLUT_KEY_LEFT : 
            ifrolled=-1;
            break;
    }
} 

void releaseSpecialKey(int key, int x, int y) 
{
    switch (key) {
        case GLUT_KEY_UP : 
            deltaMove = 0.0; 
            retard=-0.005;
            acceleration=0;
            break;
        case GLUT_KEY_DOWN : 
            deltaMove = 0.0; 
            acceleration=0;
            applied_brake=0;
            break;
        case GLUT_KEY_LEFT :
            deltaRotate = 0.0; 
            roll=0;
            ifrolled=0;
            break;
        case GLUT_KEY_RIGHT : 
            deltaRotate = 0.0; 
            roll=0;
            ifrolled=0;
            break;
    }
} 

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 400);

    glutCreateWindow("The fossil park ride");
    initRendering();

    _terrain = loadTerrain("heightmap.bmp", 20);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(handleResize);
    glutIdleFunc(update); // incremental update 
    glutIgnoreKeyRepeat(1); // ignore key repeat when holding key down
    glutKeyboardFunc(processNormalKeys); // process standard key clicks
    glutSpecialFunc(pressSpecialKey); // process special key pressed
    glutKeyboardUpFunc(KeyUp); 
    glutSpecialUpFunc(releaseSpecialKey); // process special key release
    srand(time(NULL));
    for(int i =0; i<10;i++)
        for(int j=0;j<10;j++)
        {
            Fossil *fossil = new Fossil();
            fossil->x = (rand()%119 + 1);
            fossil->z = (rand()%119 + 1);
            fossil->visible = 1;
            insert_end(fossil);
        }
    glutMainLoop();
    return 0;
}
