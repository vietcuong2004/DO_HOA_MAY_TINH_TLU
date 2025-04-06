#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>

// Biến lưu trữ góc xoay của camera
float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
int lastMouseX, lastMouseY;  // Lưu trữ vị trí cuối cùng của chuột
float zoomLevel = 10.0f;  // Ban đầu khoảng cách là 10

// Các biến để kiểm soát hướng vẫy của hai tay, hai chân:
float rightLegAngle = 0.0f, leftLegAngle = 0.0f;
float rightArmAngle = 0.0f, leftArmAngle = 0.0f;
bool rightArmSwingUp = true;
bool leftArmSwingUp = true;
bool swingLegForward = true; // Trạng thái xoay chân ra trước hoặc ra sau
bool swingArmForward = true; // Trạng thái xoay tay

// Biến để tịnh tiến Doraemon dọc trục oz và ox:
float doraemonX = 0.0f, doraemonZ = 0.0f;
bool movingForward = true;  // Điều khiển hướng di chuyển: tiến về phía trước hoặc quay lại
float rotationY = 0.0f;  // Lưu trữ góc xoay Doraemon quanh trục OY. Ban đầu Doraemon không quay

// Trạng thái di chuyển theo hình vuông
int movementState = 0;  // 0: Di chuyển theo trục Z, 1: Di chuyển theo trục X

//Biến để điều chỉnh tốc độ chạy:
float RunSpeed = 0.05f;

//Khai báo hàm chuyển động
void updateMovement();

// Hàm thiết lập ánh sáng và đổ bóng
void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_position[] = { 5.0f, 5.0f, 5.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

// Hàm vẽ trục tọa độ OXYZ với độ dài 50
void drawAxes() {
    glLineWidth(2.0f);  // Độ dày của trục
    glBegin(GL_LINES);

    // Trục X màu đỏ
    glColor3f(1.0f, 0.0f, 0.0f);  // Màu đỏ cho trục X
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(50.0f, 0.0f, 0.0f);

    // Trục Y màu xanh lá
    glColor3f(0.0f, 1.0f, 0.0f);  // Màu xanh lá cho trục Y
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 50.0f, 0.0f);

    // Trục Z màu xanh dương
    glColor3f(0.0f, 0.0f, 1.0f);  // Màu xanh dương cho trục Z
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 50.0f);

    glEnd();
}

// Hàm vẽ mặt phẳng (Làm mặt đất cho Doraemon chạy trên đó)
void drawPlane() {
    glColor3f(0.8f, 0.8f, 0.8f);  // Màu xám nhạt cho mặt phẳng
    glBegin(GL_QUADS);//Vẽ tứ giác
    glVertex3f(-5.0f, -1.5f, -5.0f);  // Đỉnh (A)
    glVertex3f(20.0f, -1.5f, -5.0f);  // Đỉnh (B)
    glVertex3f(20.0f, -1.5f, 20.0f);  // Đỉnh (C)
    glVertex3f(-5.0f, -1.5f, 20.0f);  // Đỉnh (D)
    glEnd();
}

// Hàm vẽ hình cầu
void drawSphere(float radius, float r, float g, float b) {
    glColor3f(r, g, b);
    glutSolidSphere(radius, 50, 50);
}

// Hàm vẽ hình trụ
void drawCylinder(float radius, float height, float r, float g, float b) {
    glColor3f(r, g, b);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, radius, radius, height, 50, 50);
    gluDeleteQuadric(quad);
}

//Hàm vẽ nửa hình cầu, để vẽ miệng và túi thần kì
void drawHalfSphere(float radius, float r, float g, float b) {
    glPushMatrix();

    // Thiết lập màu cho nửa hình cầu
    glColor3f(r, g, b);

    // Sử dụng mặt phẳng để cắt nửa dưới của hình cầu
    GLdouble plane[] = { 0.0, -1.0, 0.0, 0.0 };  // Mặt phẳng cắt (giữ lại phần dưới)
    glClipPlane(GL_CLIP_PLANE0, plane);        // Kích hoạt mặt phẳng cắt
    glEnable(GL_CLIP_PLANE0);
    glutSolidSphere(radius, 50, 50);

    // Tắt mặt phẳng cắt
    glDisable(GL_CLIP_PLANE0);

    glPopMatrix();
}

void drawFace() {
    // Vẽ phần mặt màu trắng nhỏ hơn và dịch chuyển về phía sau
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.09f);  // Tịnh tiến mặt về phía trước theo Oz +0.09
    drawSphere(0.7f, 1.0f, 1.0f, 1.0f);  // Mặt màu trắng nhỏ hơn
    glPopMatrix();

    // Vẽ mắt
    glPushMatrix();
    glTranslatef(-0.15f, 0.5f, 0.55f);  // Vị trí mắt trái
    drawSphere(0.15f, 1.0f, 1.0f, 1.0f);  // Màu trắng, bán kính 0.15
    glTranslatef(0.3f, 0.0f, 0.0f);  // Vị trí mắt phải
    drawSphere(0.15f, 1.0f, 1.0f, 1.0f);  // Màu trắng
    glPopMatrix();

    // Vẽ con ngươi
    glPushMatrix();
    glTranslatef(-0.15f, 0.5f, 0.65f);  // Vị trí con ngươi trái
    drawSphere(0.07f, 0.0f, 0.0f, 0.0f);  // Màu đen
    glTranslatef(0.3f, 0.0f, 0.0f);  // Vị trí con ngươi phải
    drawSphere(0.07f, 0.0f, 0.0f, 0.0f);  // Màu đen
    glPopMatrix();

    // Vẽ mũi
    glPushMatrix();
    glTranslatef(0.0f, 0.35f, 0.65f);  // Điều chỉnh vị trí mũi theo trục Y và Z
    drawSphere(0.1f, 1.0f, 0.0f, 0.0f);  // Màu đỏ
    glPopMatrix();

    // Vẽ miệng bằng nửa hình cầu đỏ
    glPushMatrix();
    //Miệng cười
    //glTranslatef(0.0f, 0.1f, 0.5f);

    //Miệng mếu:
    glTranslatef(0.0f, -0.2f, 0.5f);
    glRotatef(180.0f, 0.0f, 0.0f, 1.0f);

    drawHalfSphere(0.4f, 1.0f, 0.0f, 0.0f);  // Vẽ miệng với nửa hình cầu đỏ
    glPopMatrix();

    // Vẽ râu bằng các đường thẳng
    glPushMatrix();
    glBegin(GL_LINES);

    glColor3f(0.0f, 0.0f, 0.0f);  // Màu đen
    // Vẽ 3 râu bên trái
    glVertex3f(-0.4f, 0.25f, 0.55f); glVertex3f(-0.8f, 0.25f, 0.55f);
    glVertex3f(-0.4f, 0.35f, 0.55f); glVertex3f(-0.8f, 0.4f, 0.55f);
    glVertex3f(-0.4f, 0.15f, 0.55f); glVertex3f(-0.8f, 0.1f, 0.55f);
    // Vẽ 3 râu bên phải
    glVertex3f(0.4f, 0.25f, 0.55f); glVertex3f(0.8f, 0.25f, 0.55f);
    glVertex3f(0.4f, 0.35f, 0.55f); glVertex3f(0.8f, 0.4f, 0.55f);
    glVertex3f(0.4f, 0.15f, 0.55f); glVertex3f(0.8f, 0.1f, 0.55f);

    glEnd();
    glPopMatrix();
}

// Hàm vẽ Doraemon với các chi tiết như thân, đầu, mắt, mũi, tay, chân
void drawDoraemon() {
    // Vẽ thân trước tại gốc tọa độ
    glPushMatrix();
    drawSphere(1.0f, 0.0f, 0.5f, 1.0f);  // Thân màu xanh dương với bán kính 1
    glPopMatrix();
    //Màu trắng trước bụng
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.15f);  // Tịnh tiến mặt về phía trước theo Oz +0.09
    drawSphere(0.9f, 1.0f, 1.0f, 1.0f);  // Mặt màu trắng nhỏ hơn
    glPopMatrix();

    // Vẽ túi thần kỳ bằng nửa hình cầu màu trắng
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.5f);  // Tịnh tiến túi thần kỳ xuống vị trí bụng
    drawHalfSphere(0.6f, 0.5f, 0.5f, 0.5f);  // Túi thần kỳ màu trắng
    glPopMatrix();

    // Vẽ đầu
    glPushMatrix();
    glTranslatef(0.0f, 1.4f, 0.0f);  // Điều chỉnh đầu lên phía trên thân
    drawSphere(0.75f, 0.0f, 0.5f, 1.0f);  // Đầu màu xanh dương
    glPopMatrix();

    // Vẽ khuôn mặt
    glPushMatrix();
    glTranslatef(0.0f, 1.4f, 0.0f);  // Khuôn mặt cũng di chuyển lên theo đầu
    drawFace();
    glPopMatrix();

    // Vẽ vòng cổ màu đỏ
    glPushMatrix();
    glTranslatef(0.0f, 0.85f, 0.0f);  // Điều chỉnh vị trí vòng cổ ngay dưới đầu
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);  // Xoay hình trụ để tạo vòng cổ ngang qua cổ
    drawCylinder(0.65f, 0.09f, 1.0f, 0.0f, 0.0f);  // Vòng cổ màu đỏ, bán kính 0.65 và chiều cao 0.09
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.75f);  // Tịnh tiến vị trí chuông ngay dưới vòng cổ
    drawSphere(0.15f, 1.0f, 1.0f, 0.0f);  // Hình cầu màu vàng, bán kính 0.15
    glPopMatrix();

    //Vẽ đuôi: hình cầu màu đỏ:
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -1.0f);  // Tịnh tiến
    drawSphere(0.15f, 1.0f, 0.0f, 0.0f);  // Hình cầu màu đỏ bán kính 0.15
    glPopMatrix();

    // Vẽ tay trái
    glPushMatrix();
    glTranslatef(-0.7f, 0.5f, 0.0f);  // Dịch vị trí cánh tay trái lên để gắn vào thân
    glRotatef(leftArmAngle, 0.0f, 0.0f, 1.0f);  // Xoay tay trái với góc leftArmAngle
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);  // Xoay cánh tay trái giống cánh tay phải
    glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);  // Xoay cánh tay -15 độ quanh trục OX
    drawCylinder(0.2f, 0.6f, 0.0f, 0.5f, 1.0f);  // Cánh tay to hơn (hình trụ)
    glTranslatef(0.0f, 0.0f, 0.7f);
    drawSphere(0.3f, 1.0f, 1.0f, 1.0f);  // Bàn tay (hình cầu màu trắng)
    glPopMatrix();

    // Vẽ tay phải
    glPushMatrix();
    glTranslatef(0.7f, 0.5f, 0.0f);  // Dịch vị trí cánh tay phải lên để gắn vào thân
    glRotatef(rightArmAngle, 0.0f, 0.0f, 1.0f);  // Xoay tay phải với góc rightArmAngle (ngược với tay trái)
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);  // Xoay cánh tay 90 độ quanh trục OY
    glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);  // Xoay cánh tay quanh trục OX
    drawCylinder(0.2f, 0.6f, 0.0f, 0.5f, 1.0f);  // Cánh tay to hơn (hình trụ)
    glTranslatef(0.0f, 0.0f, 0.7f);
    drawSphere(0.3f, 1.0f, 1.0f, 1.0f);  // Bàn tay (hình cầu màu trắng)
    glPopMatrix();

    // Vẽ chân trái
    glPushMatrix();
    glTranslatef(-0.6f, -0.6f, 0.0f);  // Dịch chân trái gần về phía thân
    glRotatef(leftLegAngle, 1.0f, 0.0f, 0.0f);  // Xoay chân trái quanh trục OX
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);  // Xoay chân 90 độ quanh trục OX để về vị trí thả thõng ban đầu
    drawCylinder(0.25f, 0.6f, 0.0f, 0.5f, 1.0f);  // Phần trên của chân (hình trụ)
    glTranslatef(0.0f, 0.0f, 0.7f);
    drawSphere(0.3f, 1.0f, 1.0f, 1.0f);  // Bàn chân (hình cầu)
    glPopMatrix();

    // Vẽ chân phải
    glPushMatrix();
    glTranslatef(0.6f, -0.6f, 0.0f);  // Dịch chân phải gần về phía thân
    glRotatef(rightLegAngle, 1.0f, 0.0f, 0.0f);  // Xoay chân phải quanh trục OX
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);  // Xoay chân 90 độ quanh trục OX để về vị trí thả thõng ban đầu
    drawCylinder(0.25f, 0.6f, 0.0f, 0.5f, 1.0f);  // Phần trên của chân (hình trụ)
    glTranslatef(0.0f, 0.0f, 0.7f);
    drawSphere(0.3f, 1.0f, 1.0f, 1.0f);  // Bàn chân (hình cầu)
    glPopMatrix();

}

void updateMovement() {
    // Điều chỉnh chuyển động của chân và tay
    if (swingLegForward) {
        leftLegAngle += 1.0f;  // Chân trái xoay ra trước quanh trục OX
        rightLegAngle -= 1.0f; // Chân phải xoay ra sau quanh trục OX
        if (leftLegAngle >= 45.0f) {
            swingLegForward = false;  // Đổi hướng khi đạt giới hạn
        }
    }
    else {
        leftLegAngle -= 1.0f;  // Chân trái xoay ra sau quanh trục OX
        rightLegAngle += 1.0f; // Chân phải xoay ra trước quanh trục OX
        if (leftLegAngle <= -45.0f) {
            swingLegForward = true;  // Đổi hướng khi đạt giới hạn
        }
    }

    if (swingArmForward) {
        rightArmAngle += 1.0f;
        leftArmAngle -= 1.0f;
        if (rightArmAngle >= 45.0f) {
            swingArmForward = false;
        }
    }
    else {
        rightArmAngle -= 1.0f;
        leftArmAngle += 1.0f;
        if (rightArmAngle <= -45.0f) {
            swingArmForward = true;
        }
    }

    // Chuyển động theo hình vuông
    switch (movementState) {
    case 0:  // Di chuyển từ (0,0,0) đến (0,0,15) theo trục Z
        doraemonZ += RunSpeed;
        if (doraemonZ >= 15.0f) {
            movementState = 1;
            rotationY += 90.0f;  // Quay 90 độ quanh trục Y
        }
        break;
    case 1:  // Di chuyển từ (0,0,15) đến (15,0,15) theo trục X
        doraemonX += RunSpeed;
        if (doraemonX >= 15.0f) {
            movementState = 2;
            rotationY += 90.0f;  // Quay 90 độ quanh trục Y
        }
        break;
    case 2:  // Di chuyển từ (15,0,15) đến (15,0,0) theo trục Z
        doraemonZ -= RunSpeed;
        if (doraemonZ <= 0.0f) {
            movementState = 3;
            rotationY += 90.0f;  // Quay 90 độ quanh trục Y
        }
        break;
    case 3:  // Di chuyển từ (15,0,0) đến (0,0,0) theo trục X
        doraemonX -= RunSpeed;
        if (doraemonX <= 0.0f) {
            movementState = 0;
            rotationY += 90.0f;  // Quay 90 độ quanh trục Y
        }
        break;
    }
}

// Hàm vẽ toàn bộ cảnh
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Tính toán vị trí camera dựa trên các góc xoay và mức zoom (zoomLevel)
    float eyeX = zoomLevel * sin(angleX) * cos(angleY);
    float eyeY = zoomLevel * sin(angleY);
    float eyeZ = zoomLevel * cos(angleX) * cos(angleY);

    gluLookAt(eyeX, eyeY, eyeZ,  // Vị trí camera thay đổi theo chuột và zoomLevel
              0, 0, 0,  // Điểm mà camera nhìn vào
              0, 1, 0);   // Trục ox,oy,oz hướng lên trên

    // Cập nhật chuyển động của Doraemon
    updateMovement();

    // Thực hiện đi xung quanh: 
    // Vẽ Doraemon và thực hiện tịnh tiến theo trục X, Z và xoay quanh trục OY
    glPushMatrix();
    glTranslatef(doraemonX, 0.0f, doraemonZ);  // Tịnh tiến Doraemon theo X và Z
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);  // Xoay Doraemon quanh trục OY
    drawDoraemon();
    glPopMatrix();

    // Vẽ trục tọa độ OXYZ
  /*  glPushMatrix();
    drawAxes();
    glPopMatrix();*/

    // Vẽ mặt phẳng tại z = -1.5
    drawPlane();
    glutSwapBuffers();
}


// Hàm xử lý sự kiện bàn phím cho zoom in/zoom out
void keyboard(unsigned char key, int x, int y) {
    if (key == 'z') {
        zoomLevel += 0.5f;  // Bấm 'z' để zoom out (đưa camera ra xa)
        if (zoomLevel > 50.0f) zoomLevel = 50.0f;  // Giới hạn xa nhất
    }
    if (key == 'x') {
        zoomLevel -= 0.5f;  // Bấm 'x' để zoom in (đưa camera lại gần)
        if (zoomLevel < 2.0f) zoomLevel = 2.0f;  // Giới hạn gần nhất
    }
    glutPostRedisplay();  // Yêu cầu vẽ lại sau khi thay đổi zoom
}

// Hàm xử lý sự kiện di chuyển chuột để thay đổi góc quan sát
void mouseMotion(int x, int y) {
    // Tính toán sự thay đổi của chuột
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    // Điều chỉnh góc quay camera theo trục X, Y và Z
    angleX += dx * 0.01f;
    angleY += dy * 0.01f;

    // Giới hạn góc nhìn theo trục Y để tránh lật ngược camera
    if (angleY > 1.5f) angleY = 1.5f;
    if (angleY < -1.5f) angleY = -1.5f;

    lastMouseX = x;
    lastMouseY = y;

    glutPostRedisplay();  // Yêu cầu vẽ lại cảnh
}

// Hàm khởi tạo OpenGL
void init() {
    glEnable(GL_DEPTH_TEST);  // Kích hoạt kiểm tra độ sâu
    glClearColor(1.0, 1.0, 1.0, 1.0);  // Đặt nền màu trắng
    initLighting();  // Thiết lập ánh sáng
}

// Hàm thay đổi tỷ lệ khung hình
void reshape(int w, int h) {
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, aspect, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void TimerFunction(int value) {
    // Cập nhật chuyển động tay phải
    updateMovement();

    // Yêu cầu vẽ lại cảnh
    glutPostRedisplay();

    // Đặt lại bộ đếm thời gian để gọi lại chính nó sau 16ms (~60 FPS)
    glutTimerFunc(16, TimerFunction, 0);
}

// Hàm chính
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Doraemon 3D");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);  // Xử lý khi thay đổi kích thước cửa sổ
    glutMotionFunc(mouseMotion);  // Bắt sự kiện di chuyển chuột
    glutKeyboardFunc(keyboard);  // Thêm sự kiện bấm phím
    glutTimerFunc(16, TimerFunction, 0);
    glutMainLoop();

    return 0;
}