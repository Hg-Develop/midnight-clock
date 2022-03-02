/*
 *  作者 芳賀七海
 *  copyright (c) 2019 芳賀七海 All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "colorcode.h"

#include <GL/glut.h>
#include <GL/glpng.h>

// ===== MACRO =====
#define BACKGROUND 1, 1, 1, 0
#define Z_AXIS 0, 0, 1
#define QUADRANT (4)
#define DIAL_H (12)
#define FILENAME (255)
#define RECORDS (25)
#define WINDOW_HISTORY_ROWS (6)
#define FONT_MIN (10)

// ===== TYPE =====
typedef struct tm Time;

typedef struct image {
  GLuint  id;
  pngInfo info;
} Image;

typedef struct record {
  int y;
  int m;
  int s;
  Image *alt_main;
  Image *alt_number;
} Record;

// ===== PROTOTYPE =====
void cb_main_display(void);
void cb_main_reshape(int, int);
void cb_number_display(void);
void cb_number_reshape(int, int);
void cb_history_display(void);
void cb_history_reshape(int, int);
void cb_history_mouse(int, int, int, int);
void cb_history_pmotion(int, int);
void cb_history_entry(int);
void cb_timer_display(int);

void manage_reshape(int, int, int, int);

void generate_dial(void);
void generate_dial_number(void);
void generate_blank(const Time *);
void generate_message(const Time *, int);
void generate_hand_h(const Time *);
void generate_hand_m(const Time *);
void generate_cursor_up(void);
void generate_cursor_down(void);
void generate_choice(int);
void generate_mask();

void draw_circle(double, double, double);
void draw_rectangle(double, double, double, double);
void alpha_texture(void);

void Image_read(Image *, char *);
void Image_put(const Image *, double, double);

Time *Time_now(void);

// ===== CONSTANT =====
const double ROUND = 360.0;
const int DIAL_M = 60;
const int DIAL_S = 60;

const int INTERVAL = 50;

Record RECORD[RECORDS] = {
  { 1947, 53, 0,  NULL, NULL },
  { 1949, 57, 0,  NULL, NULL },
  { 1953, 58, 0,  NULL, NULL },
  { 1960, 53, 0,  NULL, NULL },
  { 1963, 48, 0,  NULL, NULL },
  { 1968, 53, 0,  NULL, NULL },
  { 1969, 50, 0,  NULL, NULL },
  { 1972, 48, 0,  NULL, NULL },
  { 1974, 51, 0,  NULL, NULL },
  { 1980, 53, 0,  NULL, NULL },
  { 1981, 56, 0,  NULL, NULL },
  { 1984, 57, 0,  NULL, NULL },
  { 1988, 54, 0,  NULL, NULL },
  { 1990, 50, 0,  NULL, NULL },
  { 1991, 43, 0,  NULL, NULL },
  { 1995, 46, 0,  NULL, NULL },
  { 1998, 51, 0,  NULL, NULL },
  { 2002, 53, 0,  NULL, NULL },
  { 2007, 55, 0,  NULL, NULL },
  { 2010, 54, 0,  NULL, NULL },
  { 2012, 55, 0,  NULL, NULL },
  { 2015, 57, 0,  NULL, NULL },
  { 2017, 57, 30, NULL, NULL },
  { 2018, 58, 0,  NULL, NULL },
  { 2019, 58, 0,  NULL, NULL }
};

// ===== OPTION CONSTANT =====
const char * const PATH_PNG       = "png/";
const char * const PATH_FONT_MIN  = "font/min/";
const char * const PATH_FONT_HOUR = "font/hour/";
const char * const PATH_FONT_YEAR = "font/year/";

const char * const PNG_MESSAGE  = "message";
const char * const PNG_ALT_2017 = "alt-2017";

const char * const TITLE_MAIN  = "Midnight Clock";
const char * const TITLE_NUMBER  = "Number Dial";
const char * const TITLE_HISTORY  = "History";

const int WINDOW_MAIN       = 0;
const int WINDOW_MAIN_MIN   = 256;
const int WINDOW_MAIN_MAX   = 640;
const int WINDOW_MAIN_INIT  = 640;
const int WINDOW_MAIN_X     = 0;
const int WINDOW_MAIN_Y     = 0;
const int WINDOW_NUMBER       = 1;
const int WINDOW_NUMBER_MIN   = 256;
const int WINDOW_NUMBER_MAX   = 640;
const int WINDOW_NUMBER_INIT  = 640;
const int WINDOW_HISTORY       = 2;
const int WINDOW_HISTORY_W     = 256;
const int WINDOW_HISTORY_H     = 64 * 6;
const int WINDOW_HISTORY_ROW_H = 64 * 6 / WINDOW_HISTORY_ROWS;

const double HAND_H_W = 0.02;
const double HAND_H_BODY = 0.45;
const double HAND_H_TAIL = 0.1;
const double HAND_M_W = 0.02;
const double HAND_M_BODY = 0.53;
const double HAND_M_TAIL = 0.1;

const double DIAL_R_BIG   = 0.95;
const double DIAL_R_SMALL = 0.80;
const double DIAL_CIRCLE_DIST = 0.65;
const double DIAL_CIRCLE_R    = 0.1;

const int CURSOR_W = 32;
const int CURSOR_H = 32;

const int VERTEX = 360;

// ===== GLOBAL VARIABLE =====
int windows[] = { -1, -1, -1, -1 };

int window_main_size;
int window_main_x;
int window_main_y;

Image img_main_message;
Image img_main_min[FONT_MIN];
Image img_main_alt_2017;
Image img_number_message;
Image img_number_min[FONT_MIN];
Image img_number_alt_2017;
Image img_number_number[DIAL_H];
Image img_history_year[RECORDS];

int history_mouse_y;
int history_offset;
Record *history_choosen;

// ===== MAIN =====
int main(int argc, char **argv) {
  glutInit(&argc, argv);

  { /* === main window === */
    glutInitWindowSize(WINDOW_MAIN_INIT, WINDOW_MAIN_INIT);
    glutInitWindowPosition(WINDOW_MAIN_X, WINDOW_MAIN_Y);

    windows[WINDOW_MAIN] = glutCreateWindow(TITLE_MAIN);

    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA);
    glClearColor(BACKGROUND);

    glutDisplayFunc(cb_main_display);
    glutReshapeFunc(cb_main_reshape);

    { /* === read image === */
      int i;
      char filename[FILENAME];

      alpha_texture();

      sprintf(filename, "%s%s.png", PATH_PNG, PNG_MESSAGE);
      Image_read(&img_main_message, filename);

      for (i = 0; i < FONT_MIN; i++) {
        sprintf(filename, "%s%d.png", PATH_FONT_MIN, i);
        Image_read(&(img_main_min[i]), filename);
      }

      sprintf(filename, "%s%s.png", PATH_PNG, PNG_ALT_2017);
      Image_read(&img_main_alt_2017, filename);
    }

    window_main_size = glutGet(GLUT_WINDOW_WIDTH);
    window_main_x = glutGet(GLUT_WINDOW_X);
    window_main_y = glutGet(GLUT_WINDOW_Y);

    RECORD[22].alt_main = &img_main_alt_2017;
  }

  { /* === number window === */
    glutInitWindowSize(WINDOW_NUMBER_INIT, WINDOW_NUMBER_INIT);
    glutInitWindowPosition(WINDOW_MAIN_X + WINDOW_MAIN_INIT / 4, WINDOW_MAIN_Y + WINDOW_MAIN_INIT / 4);

    windows[WINDOW_NUMBER] = glutCreateWindow(TITLE_NUMBER);

    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA);
    glClearColor(BACKGROUND);

    glutDisplayFunc(cb_number_display);
    glutReshapeFunc(cb_number_reshape);

    { /* === read image === */
      int i = 0;
      char filename[FILENAME];

      alpha_texture();

      sprintf(filename, "%s%s.png", PATH_PNG, PNG_MESSAGE);
      Image_read(&img_number_message, filename);

      sprintf(filename, "%s%d.png", PATH_FONT_HOUR, 12);
      Image_read(&(img_number_number[i]), filename);
      for (i = 1; i < DIAL_H; i++) {
        sprintf(filename, "%s%d.png", PATH_FONT_HOUR, i);
        Image_read(&(img_number_number[i]), filename);
      }

      for (i = 0; i < FONT_MIN; i++) {
        sprintf(filename, "%s%d.png", PATH_FONT_MIN, i);
        Image_read(&(img_number_min[i]), filename);
      }

      sprintf(filename, "%s%s.png", PATH_PNG, PNG_ALT_2017);
      Image_read(&img_number_alt_2017, filename);
    }

    RECORD[22].alt_number = &img_number_alt_2017;
  }

  { /* === history window  === */
    int i;
    char filename[FILENAME];

    glutInitWindowSize(WINDOW_HISTORY_W, WINDOW_HISTORY_H);
    glutInitWindowPosition(WINDOW_MAIN_X + WINDOW_MAIN_INIT * 5 / 4, WINDOW_MAIN_Y);

    windows[WINDOW_HISTORY] = glutCreateWindow(TITLE_HISTORY);

    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA);
    glClearColor(BACKGROUND);

    glutDisplayFunc(cb_history_display);
    glutReshapeFunc(cb_history_reshape);
    glutMouseFunc(cb_history_mouse);
    glutPassiveMotionFunc(cb_history_pmotion);
    glutEntryFunc(cb_history_entry);

    { /* === read image === */
      alpha_texture();

      for (i = 0; i < RECORDS; i++) {
        sprintf(filename, "%s%d.png", PATH_FONT_YEAR, RECORD[i].y);
        Image_read(&(img_history_year[i]), filename);
      }
    }

    history_mouse_y = -1;
    history_offset  = RECORDS - WINDOW_HISTORY_ROWS + 2;
    history_choosen = NULL;
  }

  glutTimerFunc(INTERVAL, cb_timer_display, 0);

  glutMainLoop();

  return 0;
}

// ===== CALLBACK =====
void cb_main_display(void) {
  Time * const t = Time_now();

  if (history_choosen != NULL) {
    t->tm_hour = 23;
    t->tm_min  = history_choosen->m;
    t->tm_sec  = history_choosen->s;
  }

  if (t->tm_sec >= 60) {
    t->tm_sec = 59;
  }

  glClear(GL_COLOR_BUFFER_BIT);
  { /* === generate === */
    generate_dial();
    generate_blank(t);
    generate_message(t, 1);
    generate_hand_h(t);
    generate_hand_m(t);
  }
  glFlush();
}

void cb_main_reshape(int w, int h) {
  manage_reshape(w, h, WINDOW_MAIN_MIN, WINDOW_MAIN_MAX);
}

void cb_number_display(void) {
  const int SIZE = glutGet(GLUT_WINDOW_WIDTH);
  const int X = glutGet(GLUT_WINDOW_X);
  const int Y = glutGet(GLUT_WINDOW_Y);
  const double SCALE = (double)window_main_size / SIZE;

  Time * const t = Time_now();
  //t->tm_hour = 23;
  //t->tm_min  = 55;
  //t->tm_sec  = 0;

  if (history_choosen != NULL) {
    t->tm_hour = 23;
    t->tm_min  = history_choosen->m;
    t->tm_sec  = history_choosen->s;
  }

  if (t->tm_sec >= 60) {
    t->tm_sec = 59;
  }

  glClear(GL_COLOR_BUFFER_BIT);
  glPushMatrix();
    glTranslated(
      ((window_main_x + window_main_size / 2) - (X + SIZE / 2)) * 2.0 / SIZE,
      ((Y  + SIZE / 2) - (window_main_y + window_main_size / 2)) * 2.0 / SIZE,
      0
    );
    glScaled(SCALE, SCALE, 1);
    { /* === generate === */
      generate_dial_number();
      generate_blank(t);
      generate_message(t, 0);
      generate_hand_h(t);
      generate_hand_m(t);
    }
  glPopMatrix();
  glFlush();
}

void cb_number_reshape(int w, int h) {
  manage_reshape(w, h, WINDOW_NUMBER_MIN, WINDOW_NUMBER_MAX);
}

void cb_history_display(void) {
  int idx;

  glClear(GL_COLOR_BUFFER_BIT);
  {
    generate_cursor_up();
    for (idx = 0; idx < WINDOW_HISTORY_ROWS - 2; idx++) {
      generate_choice(idx);
    }
    generate_cursor_down();
    generate_mask();
  }
  glFlush();
}

void cb_history_reshape(int w, int h) {
  w = WINDOW_HISTORY_W;
  h = WINDOW_HISTORY_H;

  glViewport(0, 0, w, h);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);
  glScaled(1, -1, 1);
  glTranslated(0, -h, 0);

  glutReshapeWindow(w, h);
}

void cb_history_mouse(int b, int s, int x, int y) {
  if (s != GLUT_DOWN) {
    history_choosen = NULL;
    return;
  }

  switch (y / WINDOW_HISTORY_ROW_H) {
    case 0: {
      if (history_offset > 0) {
        history_offset--;
      }
      break;
    }
    case WINDOW_HISTORY_ROWS - 1: {
      if (history_offset < RECORDS - WINDOW_HISTORY_ROWS + 2) {
        history_offset++;
      }
      break;
    }
    default: {
      history_choosen = &(RECORD[
        history_offset + y / WINDOW_HISTORY_ROW_H - 1
      ]);
      break;
    }
  }
}

void cb_history_pmotion(int x, int y) {
  history_mouse_y = y;
}

void cb_history_entry(int s) {
  if (s == GLUT_LEFT) {
    history_mouse_y = -1;
  }
}

void cb_timer_display(int val) {
  int i = 0;

  glutSetWindow(windows[WINDOW_MAIN]);
  window_main_size = glutGet(GLUT_WINDOW_WIDTH);
  window_main_x = glutGet(GLUT_WINDOW_X);
  window_main_y = glutGet(GLUT_WINDOW_Y);

  while (windows[i] > -1) {
    glutSetWindow(windows[i]);
    glutPostRedisplay();
    i++;
  }

  glutTimerFunc(INTERVAL, cb_timer_display, 0);
}

// ===== MANAGE =====
void manage_reshape(int w, int h, int min, int max) {
  if (w < min) {
    w = min;
  }
  else if (w > max) {
    w = max;
  }

  if (h < min) {
    h = min;
  }
  else if (h > max) {
    h = max;
  }

  if (w > h) {
    w = h;
  }
  else {
    h = w;
  }

  glViewport(0, 0, w, h);
  glutReshapeWindow(w, h);
}

// ===== GENERATE =====
void generate_dial(void) {
  int i;

  glColor3ub(CC_BLACK);
  draw_circle(0, 0, DIAL_R_BIG);
  glColor3ub(CC_WHITE);
  draw_circle(0, 0, DIAL_R_SMALL);

  glColor3ub(CC_BLACK);
  glPushMatrix();
    for (i = 0; i < DIAL_H; i++) {
      draw_circle(0, -DIAL_CIRCLE_DIST, DIAL_CIRCLE_R);
      glRotated(-(ROUND / DIAL_H), Z_AXIS);
    }
  glPopMatrix();
}

void generate_dial_number(void) {
  const double SCALE_IMAGE = (double)glutGet(GLUT_WINDOW_WIDTH) / WINDOW_MAIN_MAX;
  const double SCALE_COORD = 2.0 / glutGet(GLUT_WINDOW_WIDTH);

  int i;

  glColor3ub(CC_BLACK);
  draw_circle(0, 0, DIAL_R_BIG);
  glColor3ub(CC_WHITE);
  draw_circle(0, 0, DIAL_R_SMALL);

  glColor3ub(CC_BLACK);
  glPushMatrix();
    for (i = 0; i < DIAL_H; i++) {
      glPushMatrix();
        glTranslated(
          -(DIAL_CIRCLE_R * 2),
          DIAL_CIRCLE_DIST - DIAL_CIRCLE_R,
          0
        );
        glScaled(SCALE_IMAGE, SCALE_IMAGE, 1);
        glScaled(SCALE_COORD, SCALE_COORD, 1);
        Image_put(&(img_number_number[i]), 0, 0);
      glPopMatrix();
      glRotated(-(ROUND / DIAL_H), Z_AXIS);
    }
  glPopMatrix();
}

void generate_blank(const Time *t) {
  int i;
  int quadrant[QUADRANT];

  for (i = 0; i < QUADRANT; i++) {
    quadrant[i] = 0;
  }

  if (history_choosen == NULL) {
    quadrant[(t->tm_hour % DIAL_H) / 3] = 1;
    quadrant[(t->tm_min  / (DIAL_M / DIAL_H)) / 3] = 1;
  }
  else {
    quadrant[QUADRANT - 1] = 1;
  }

  glColor3ub(CC_WHITE);
  for (i = 0; i < QUADRANT; i++) {
    const int NEXT = i == QUADRANT - 1 ? 0 : i + 1;

    if (quadrant[i]) {
      continue;
    }

    glPushMatrix();
      glRotated(
        -(ROUND / QUADRANT * i),
        Z_AXIS
      );

      draw_rectangle(
        DIAL_CIRCLE_R,
        (quadrant[NEXT] ? 1 : -1) * (DIAL_CIRCLE_R),
        1 - (DIAL_CIRCLE_R),
        1 + (quadrant[NEXT] ? -1 : 1) * (DIAL_CIRCLE_R)
      );
    glPopMatrix();
  }
}

void generate_message(const Time *t, int main) {
  const double SCALE_IMAGE = (glutGet(GLUT_WINDOW_WIDTH) * (1 + DIAL_CIRCLE_R) / 2) / img_main_message.info.Width;
  const double SCALE_COORD = 2.0 / glutGet(GLUT_WINDOW_WIDTH);

  if (history_choosen == NULL && (t->tm_hour != 23 || t->tm_min < 45)) {
    return;
  }

  glPushMatrix();
    glTranslated(
      -1,
      -(DIAL_CIRCLE_R + img_main_message.info.Height * SCALE_IMAGE * SCALE_COORD),
      0
    );
    glScaled(SCALE_IMAGE, SCALE_IMAGE, 1);
    glScaled(SCALE_COORD, SCALE_COORD, 1);
    Image_put(main ? &img_main_message : &img_number_message, 0, 0);
  glPopMatrix();

  glPushMatrix();
    glTranslated(
      -0.46,
      -0.335,
      0
    );
    glScaled(SCALE_IMAGE, SCALE_IMAGE, 1);
    glScaled(SCALE_COORD, SCALE_COORD, 1);
    if (main && history_choosen != NULL && history_choosen->alt_main != NULL) {
      Image_put(history_choosen->alt_main, 0, 0);
    }
    else if (!main && history_choosen != NULL && history_choosen->alt_number != NULL) {
      Image_put(history_choosen->alt_number, 0, 0);
    }
    else {
      if ((60 - t->tm_min) / 10) {
        Image_put(
          main ? &(img_main_min[(60 - t->tm_min) / 10]) : &(img_number_min[(60 - t->tm_min) / 10]),
          0, 0
        );
      }

      glTranslated(
        img_main_min[0].info.Width * 0.7,
        0,
        0
      );
      Image_put(
        main ? &(img_main_min[(60 - t->tm_min) % 10]) : &(img_number_min[(60 - t->tm_min) % 10]),
        0, 0
      );
    }
  glPopMatrix();
}

void generate_hand_h(const Time *t) {
  glPushMatrix();
    glRotated(
      -(
        ROUND / DIAL_H * t->tm_hour
        + ROUND / DIAL_H / DIAL_M * t->tm_min
      ),
      Z_AXIS
    );

    glColor3ub(CC_BLACK);
    draw_rectangle(
      -HAND_H_W,
      -HAND_H_TAIL,
      HAND_H_W * 2,
      HAND_H_TAIL + HAND_H_BODY
    );
  glPopMatrix();
}

void generate_hand_m(const Time *t) {
  glPushMatrix();
    glRotated(
      -(
        ROUND / DIAL_M * t->tm_min
        + ROUND / DIAL_M / DIAL_S * t->tm_sec
      ),
      Z_AXIS
    );

    glColor3ub(CC_BLACK);
    draw_rectangle(
      -HAND_M_W,
      -HAND_M_TAIL,
      HAND_M_W * 2,
      HAND_M_TAIL + HAND_M_BODY
    );
  glPopMatrix();
}

void generate_cursor_up() {
  if (history_offset == 0) {
    return;
  }

  glPushMatrix();
    glTranslated(
      WINDOW_HISTORY_W / 2,
      WINDOW_HISTORY_ROW_H / 2,
      0
    );

    glColor3ub(CC_BLACK);
    glBegin(GL_POLYGON);
      glVertex2d(-(CURSOR_W / 2), CURSOR_H / 2);
      glVertex2d(CURSOR_W / 2, CURSOR_H / 2);
      glVertex2d(0, -(CURSOR_H / 2));
    glEnd();
  glPopMatrix();
}

void generate_cursor_down(void) {
  if (history_offset == RECORDS - WINDOW_HISTORY_ROWS + 2) {
    return;
  }

  glPushMatrix();
    glTranslated(
      WINDOW_HISTORY_W / 2,
      WINDOW_HISTORY_H - WINDOW_HISTORY_ROW_H / 2,
      0
    );

    glColor3ub(CC_BLACK);
    glBegin(GL_POLYGON);
      glVertex2d(-(CURSOR_W / 2), -(CURSOR_H / 2));
      glVertex2d(CURSOR_W / 2, -(CURSOR_H / 2));
      glVertex2d(0, CURSOR_H / 2);
    glEnd();
  glPopMatrix();
}

void generate_choice(int idx) {
  glPushMatrix();
    glTranslated(0, WINDOW_HISTORY_ROW_H * (idx + 2), 0);
    glScaled(1, -1, 1);
    Image_put(&(img_history_year[history_offset + idx]), 0, 0);
  glPopMatrix();
}

void generate_mask() {
  const int idx = history_mouse_y / WINDOW_HISTORY_ROW_H;

  if (history_mouse_y < 0) {
    return;
  }

  glColor4ub(CC_BLACK, 255 / 2);

  if (history_offset == 0 && idx == 0) {
    return;
  }
  else if (history_offset == RECORDS - WINDOW_HISTORY_ROWS + 2 && idx == WINDOW_HISTORY_ROWS - 1) {
    return;
  }

  draw_rectangle(
    0,
    idx * WINDOW_HISTORY_ROW_H,
    WINDOW_HISTORY_W,
    WINDOW_HISTORY_ROW_H
  );
}

// ===== DRAW =====
void draw_circle(double x, double y, double r) {
  int i;

  glBegin(GL_POLYGON);
    for (i = 0; i < VERTEX; i++) {
      glVertex2d(
        x + cos(M_PI * 2 / VERTEX * i) * r,
        y + sin(M_PI * 2 / VERTEX * i) * r
      );
    }
  glEnd();
}

void draw_rectangle(double x, double y, double w, double h) {
  glBegin(GL_QUADS);
    glVertex2d(x, y);
    glVertex2d(x, y + h);
    glVertex2d(x + w, y + h);
    glVertex2d(x + w, y);
  glEnd();
}

// ===== IMAGE =====
void Image_read(Image *img, char *filename) {
  img->id = pngBind(
    filename,
    PNG_NOMIPMAP,
    PNG_ALPHA,
    &(img->info),
    GL_CLAMP,
    GL_NEAREST,
    GL_NEAREST
  );
}

void Image_put(const Image *img, double x, double y) {
  const double W = (double)img->info.Width;
  const double H = (double)img->info.Height;

  glPushMatrix();
    glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, img->id);
      glColor4ub(255, 255, 255, 255);

      glBegin(GL_QUADS);
        glTexCoord2i(0, 0);
        glVertex2d(x, y + H);

        glTexCoord2i(0, 1);
        glVertex2d(x, y);

        glTexCoord2i(1, 1);
        glVertex2d(x + W, y);

        glTexCoord2i(1, 0);
        glVertex2d(x + W, y + H);
      glEnd();
    glDisable(GL_TEXTURE_2D);
  glPopMatrix();
}

// ===== OTHER =====
Time *Time_now(void) {
  time_t t;
  Time *now;

  t = time(NULL);
  now = localtime(&t);

  return now;
}

void alpha_texture(void) {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
