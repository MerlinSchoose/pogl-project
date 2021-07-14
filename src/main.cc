#include <iostream>
#include <vector>

#include "opengl.hh"
#include "object_vbo.hh"
#include "image_io.hh"
#include "texload.hh"
#include "shaders.hh"

#define CAUSTICS_SIZE 512

//#define SAVE_RENDER

#define TEST_OPENGL_ERROR()                                                             \
  do {									\
    GLenum err = glGetError();					                        \
    if (err != GL_NO_ERROR) std::cerr << "OpenGL ERROR! " << __LINE__ << " " << gluErrorString(err) << std::endl;      \
  } while(0)

GLuint floor_vao_id;
GLuint surface_vao_id;
GLuint background_vao_id;
program *main_program;
program *background_program;

GLuint caustic_idx = 0;
GLuint caustic_begin = 1;
GLuint timer = 1000 / 60;

GLuint blue_texture_id;
GLuint floor_texture_id;

glm::vec3 camera_pos = glm::vec3(0.0f, -80.0f,  0.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f,  0.0f);
float camera_speed = 3.0f;

void window_resize(int width, int height) {
    glViewport(0,0,width,height);TEST_OPENGL_ERROR();
}

#if defined(SAVE_RENDER)
bool saved = false;
#endif

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();
    main_program->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floor_texture_id);
    glBindVertexArray(floor_vao_id);TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, floor_vbo.size());TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, blue_texture_id);TEST_OPENGL_ERROR();
    glBindVertexArray(surface_vao_id);TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, surface_vbo.size());TEST_OPENGL_ERROR();
    glBindVertexArray(0);TEST_OPENGL_ERROR();
    background_program->use();
    glBindVertexArray(background_vao_id);TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, background_vbo.size());TEST_OPENGL_ERROR();
    main_program->use();

    glutSwapBuffers();
}

void idleFunc() {
    caustic_idx = (caustic_idx + 1) % CAUSTICS_SIZE;
    glActiveTexture(GL_TEXTURE1);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, caustic_idx + caustic_begin);TEST_OPENGL_ERROR();
    glutPostRedisplay();TEST_OPENGL_ERROR();
}

void timerFunc(int value) {
    caustic_idx = (caustic_idx + 1) % CAUSTICS_SIZE;
    glActiveTexture(GL_TEXTURE1);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, caustic_idx + caustic_begin);TEST_OPENGL_ERROR();
    glutPostRedisplay();TEST_OPENGL_ERROR();
    glutTimerFunc(timer, timerFunc, value);TEST_OPENGL_ERROR();
}

void keyboardFunc(unsigned char c, int x, int y) {
    switch (c) {
        // W to move forward.
        case 'w':
            camera_pos += camera_front * camera_speed;
            break;
        // S to move backward.
        case 's':
            camera_pos -= camera_front * camera_speed;
            break;

        // A to straff left.
        case 'a':
            camera_pos -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
            break;
        // d to straff right.
        case 'd':
            camera_pos += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
            break;

        // Escape button to exit.
        case 27:
            exit(0);
            break;
    }

    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat4 view_matrix = glm::lookAt(
            camera_pos,
            camera_pos + camera_front,
            camera_up
    );

    glm::mat4 model_view_matrix = view_matrix * model_matrix;

    GLuint mv_loc = glGetUniformLocation(main_program->id, "model_view_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_view_matrix[0][0]);TEST_OPENGL_ERROR();

    GLuint cp_loc = glGetUniformLocation(main_program->id, "cameraPos");TEST_OPENGL_ERROR();
    glUniform3f(cp_loc, camera_pos.x, camera_pos.y, camera_pos.z);TEST_OPENGL_ERROR();
}

void keyboardSpecialFunc(int c, int x, int y) {
    switch (c) {
        // Up key to rotate the camera upward.
        case GLUT_KEY_UP:
            camera_front = glm::rotate(camera_front, 0.1f, glm::cross(camera_front, glm::vec3(0.0f, 1.0f, 0.0f)));
            break;
        // Down key to rotate the camera downward.
        case GLUT_KEY_DOWN:
            camera_front = glm::rotate(camera_front, -0.1f, glm::cross(camera_front, glm::vec3(0.0f, 1.0f, 0.0f)));
            break;

        // Left key to rotate the camera to the left.
        case GLUT_KEY_LEFT:
            camera_front = glm::rotate(camera_front, 0.1f * camera_speed, glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        // Right key to rotate the camera to the right.
        case GLUT_KEY_RIGHT:
            camera_front = glm::rotate(camera_front, -0.1f * camera_speed, glm::vec3(0.0f, 1.0f, 0.0f));
            break;
    }

    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat4 view_matrix = glm::lookAt(
            camera_pos,
            camera_pos + camera_front,
            camera_up
    );

    glm::mat4 model_view_matrix = view_matrix * model_matrix;

    main_program->use();
    GLuint mv_loc = glGetUniformLocation(main_program->id, "model_view_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_view_matrix[0][0]);TEST_OPENGL_ERROR();
    background_program->use();
    mv_loc = glGetUniformLocation(background_program->id, "model_view_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_view_matrix[0][0]);TEST_OPENGL_ERROR();
}

void init_glut(int &argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitContextVersion(4,6);
    glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);

    glutInitWindowSize(1920, 1080);
    glutInitWindowPosition ( 100, 100 );
    glutCreateWindow("Underwater Scene");

    glutDisplayFunc(display);
    glutReshapeFunc(window_resize);

    glutTimerFunc(0, timerFunc, 0);
    glutKeyboardFunc(keyboardFunc);
    glutSpecialFunc(keyboardSpecialFunc);

    glutReportErrors();
}

bool init_glew() {
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if (error != GLEW_OK) {
        std::cerr << " Error while initializing glew: " << glewGetErrorString(error) << std::endl;
        return false;
    }
    return true;
}

void init_GL() {
    glEnable(GL_DEPTH_TEST);TEST_OPENGL_ERROR();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);TEST_OPENGL_ERROR();
    glClearColor(0.0,0.35,0.75,1.0);TEST_OPENGL_ERROR();
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glPixelStorei(GL_PACK_ALIGNMENT,1);
}

void init_object_vbo() {
    int max_nb_vbo = 5;
    int nb_vbo = 0;
    int index_vbo = 0;
    GLuint vbo_ids[max_nb_vbo];

    GLint vertex_location = glGetAttribLocation(main_program->id, "position");TEST_OPENGL_ERROR();
    GLint uv_location = glGetAttribLocation(main_program->id, "uv");TEST_OPENGL_ERROR();

    // floor
    glGenVertexArrays(1, &floor_vao_id);TEST_OPENGL_ERROR();
    glBindVertexArray(floor_vao_id);TEST_OPENGL_ERROR();

    if (vertex_location!=-1) nb_vbo++;
    if (uv_location!=-1) nb_vbo++;

    glGenBuffers(nb_vbo, vbo_ids);TEST_OPENGL_ERROR();

    if (vertex_location!=-1) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[index_vbo++]);TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, floor_vbo.size()*sizeof(float),floor_vbo.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
        glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(vertex_location);TEST_OPENGL_ERROR();
    }

    if (uv_location!=-1) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[index_vbo++]);TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, uv_buffer_data.size()*sizeof(float), uv_buffer_data.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
        glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, 0);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(uv_location);TEST_OPENGL_ERROR();
    }
    // surface

    glGenVertexArrays(1, &surface_vao_id);TEST_OPENGL_ERROR();
    glBindVertexArray(surface_vao_id);TEST_OPENGL_ERROR();
    nb_vbo = 0;
    index_vbo = 0;
    if (vertex_location!=-1) nb_vbo++;
    if (uv_location!=-1) nb_vbo++;

    glGenBuffers(nb_vbo, vbo_ids);TEST_OPENGL_ERROR();

    if (vertex_location!=-1) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[index_vbo++]);TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, surface_vbo.size()*sizeof(float),surface_vbo.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
        glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(vertex_location);TEST_OPENGL_ERROR();
    }

    if (uv_location!=-1) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[index_vbo++]);TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, uv_buffer_data.size()*sizeof(float), uv_buffer_data.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
        glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, 0);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(uv_location);TEST_OPENGL_ERROR();
    }

    glBindVertexArray(0);
}

void init_background_vao() {
    background_program->use();
    GLint vertex_location = glGetAttribLocation(background_program->id, "position");TEST_OPENGL_ERROR();
    glGenVertexArrays(1, &background_vao_id);TEST_OPENGL_ERROR();
    glBindVertexArray(background_vao_id);TEST_OPENGL_ERROR();

    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);TEST_OPENGL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);TEST_OPENGL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, background_vbo.size()*sizeof(float),background_vbo.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
    glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
    glEnableVertexAttribArray(vertex_location);TEST_OPENGL_ERROR();
    glBindVertexArray(0);TEST_OPENGL_ERROR();
}

void init_uniform(GLuint program_id) {
    glm::mat4 projection_matrix = glm::perspective(
            glm::radians(50.0f),
            16.f/9.f,
            5.f, 17500.0f
    );

    GLuint mp_loc = glGetUniformLocation(program_id, "projection_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mp_loc, 1, GL_FALSE, &projection_matrix[0][0]);TEST_OPENGL_ERROR();

    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat4 view_matrix = glm::lookAt(
            camera_pos,
            camera_pos + camera_front,
            camera_up
    );

    glm::mat4 model_view_matrix = view_matrix * model_matrix;

    GLuint mv_loc = glGetUniformLocation(program_id, "model_view_matrix");TEST_OPENGL_ERROR();
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_view_matrix[0][0]);TEST_OPENGL_ERROR();

    GLuint cp_loc = glGetUniformLocation(program_id, "cameraPos");TEST_OPENGL_ERROR();
    glUniform3f(cp_loc, camera_pos.x, camera_pos.y, camera_pos.z);TEST_OPENGL_ERROR();
}

void init_textures() {
    int width, height;
    auto *caustics_id = new GLuint[CAUSTICS_SIZE];
    GLint tex_location;
    GLint texture_units, combined_texture_units;

    // Floor texture.
    GLubyte *floor_texture = tifo::load_image("../image_test/sand-texture-seamless.tga", &width, &height)->get_buffer();

    std::cout << "floor texture " << width << ", " <<  height << "\n";

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &combined_texture_units);
    std::cout << "Limit 1 " <<  texture_units << " limit 2 " << combined_texture_units << std::endl;

    glGenTextures(1, &floor_texture_id);TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, floor_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, floor_texture);TEST_OPENGL_ERROR();
    glGenerateMipmap(GL_TEXTURE_2D);

    tex_location = glGetUniformLocation(main_program->id, "floor_sampler");TEST_OPENGL_ERROR();
    glUniform1i(tex_location, 0);TEST_OPENGL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);TEST_OPENGL_ERROR();

    delete floor_texture;
    // Caustics textures.
    glGenTextures(CAUSTICS_SIZE, caustics_id);TEST_OPENGL_ERROR();
    caustic_begin = caustics_id[0];
    char *filename = new char[64];
    for (unsigned i = 0; i < CAUSTICS_SIZE; ++i) {
        std::sprintf(filename, "../image_test/caustics_v3/CausticsRender_%03d.tga", i + 1);
        auto *caustic_texture = tifo::load_gray_image(filename, &width, &height)->get_buffer();

        glActiveTexture(GL_TEXTURE1);TEST_OPENGL_ERROR();
        glBindTexture(GL_TEXTURE_2D, caustics_id[i]);TEST_OPENGL_ERROR();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, caustic_texture);TEST_OPENGL_ERROR();
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);TEST_OPENGL_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);TEST_OPENGL_ERROR();
        delete caustic_texture;
    }
    delete[] filename;

    glActiveTexture(GL_TEXTURE1);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, caustic_idx + 1);TEST_OPENGL_ERROR();
    tex_location = glGetUniformLocation(main_program->id, "caustic_sampler");TEST_OPENGL_ERROR();
    glUniform1i(tex_location, 1);TEST_OPENGL_ERROR();

    unsigned char blue[3] = {0, 89, 191};

    glGenTextures(1, &blue_texture_id);TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, blue_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, blue);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);TEST_OPENGL_ERROR();

    delete[] caustics_id;
}

bool init_shaders() {
    main_program = program::make_program("../src/vertex.vert", "../src/fragment.frag");
    if (!main_program->is_ready()) {
        std::cerr << "Main Program Creation Failed:" << main_program->get_log();
        return false;
    }
    background_program = program::make_program("../src/background.vert", "../src/background.frag");
    if (!background_program->is_ready()) {
        std::cerr << "Background Program Creation Failed:\n" << background_program->get_log() << '\n';
        return false;
    }
    main_program->use();
    return true;
}

int main(int argc, char *argv[]) {
    init_glut(argc, argv);
    if (!init_glew())
        std::exit(1);

    init_GL();

    init_shaders();
    init_uniform(main_program->id);
    init_object_vbo();
    init_textures();
    background_program->use();
    init_uniform(background_program->id);
    init_background_vao();
    main_program->use();

    glutMainLoop();
}