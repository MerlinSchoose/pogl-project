#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/ext.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include <fstream>
#include <vector>

#include "object_vbo.hh"
#include "image_io.hh"
#include "texload.hh"

#define CAUSTICS_SIZE 256

//#define SAVE_RENDER

#define TEST_OPENGL_ERROR()                                                             \
  do {									\
    GLenum err = glGetError();					                        \
    if (err != GL_NO_ERROR) std::cerr << "OpenGL ERROR! " << __LINE__ << " " << glewGetErrorString(err) << std::endl;      \
  } while(0)


GLuint floor_vao_id;
GLuint surface_vao_id;
GLuint program_id;

GLuint caustic_idx = 0;
GLuint caustic_begin = 1;
GLuint timer = 1000 / 120;

GLuint blue_texture_id;
GLuint floor_texture_id;

glm::vec3 camera_pos = glm::vec3(0.0f, -70.0f,  0.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f,  0.0f);
float camera_speed = 1.0f;

void window_resize(int width, int height) {
    glViewport(0,0,width,height);TEST_OPENGL_ERROR();
}

#if defined(SAVE_RENDER)
bool saved = false;
#endif

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floor_texture_id);
    glBindVertexArray(floor_vao_id);TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, floor_vbo.size());TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, blue_texture_id);TEST_OPENGL_ERROR();
    glBindVertexArray(surface_vao_id);TEST_OPENGL_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, surface_vbo.size());TEST_OPENGL_ERROR();
    glBindVertexArray(0);TEST_OPENGL_ERROR();

#if defined(SAVE_RENDER)
    if (!saved) {
    tifo::rgb24_image *texture = new tifo::rgb24_image(800, 590);
    glReadPixels(150, 350, 800, 590, GL_RGB, GL_UNSIGNED_BYTE, texture->pixels);TEST_OPENGL_ERROR();
    //glReadPixels(0, 0, 1024, 1024, GL_RGB, GL_UNSIGNED_BYTE, texture->pixels);
    tifo::save_image(*texture, "render.tga");
    std::cout << "Save " << std::endl;
    delete texture;
    //saved = true;
  }
#endif

    glutSwapBuffers();
}

void idleFunc() {
    caustic_idx = (caustic_idx + 1) % CAUSTICS_SIZE;
    glActiveTexture(GL_TEXTURE1);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, caustic_idx + caustic_begin);TEST_OPENGL_ERROR();
    glutPostRedisplay();
}

void timerFunc(int value) {
    caustic_idx = (caustic_idx + 1) % CAUSTICS_SIZE;
    glActiveTexture(GL_TEXTURE1);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, caustic_idx + caustic_begin);TEST_OPENGL_ERROR();
    glutPostRedisplay();
    glutTimerFunc(timer, timerFunc, value);
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

    GLuint mv_loc = glGetUniformLocation(program_id, "model_view_matrix");
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_view_matrix[0][0]);
}

void keyboardSpecialFunc(int c, int x, int y) {
    switch (c) {
        // Up key to rotate the camera upward.
        case GLUT_KEY_UP:
            camera_front = glm::rotate(camera_front, 0.1f * camera_speed, glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        // Down key to rotate the camera downward.
        case GLUT_KEY_DOWN:
            camera_front = glm::rotate(camera_front, -0.1f * camera_speed, glm::vec3(1.0f, 0.0f, 0.0f));
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

    GLuint mv_loc = glGetUniformLocation(program_id, "model_view_matrix");
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_view_matrix[0][0]);
}

void init_glut(int &argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitContextVersion(4,6);
    glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);

    glutInitWindowSize(1024, 1024);
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
    glClearColor(0.1,0.55,0.7,1.0);TEST_OPENGL_ERROR();
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glPixelStorei(GL_PACK_ALIGNMENT,1);
}

void init_object_vbo() {
    int max_nb_vbo = 5;
    int nb_vbo = 0;
    int index_vbo = 0;
    GLuint vbo_ids[max_nb_vbo];

    GLint vertex_location = glGetAttribLocation(program_id,"position");TEST_OPENGL_ERROR();
    GLint uv_location = glGetAttribLocation(program_id,"uv");TEST_OPENGL_ERROR();

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

void init_uniform() {
    glm::mat4 projection_matrix = glm::perspective(
            glm::radians(40.0f),
            1.0f,
            5.f, 1500.0f
    );

    GLuint mp_loc = glGetUniformLocation(program_id, "projection_matrix");
    glUniformMatrix4fv(mp_loc, 1, GL_FALSE, &projection_matrix[0][0]);

    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat4 view_matrix = glm::lookAt(
            camera_pos,
            camera_pos + camera_front,
            camera_up
    );

    glm::mat4 model_view_matrix = view_matrix * model_matrix;

    GLuint mv_loc = glGetUniformLocation(program_id, "model_view_matrix");
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, &model_view_matrix[0][0]);
}

void init_textures() {
    int width, height;
    auto *caustics_id = new GLuint[CAUSTICS_SIZE];
    GLint tex_location;
    GLint texture_units, combined_texture_units;

    // Floor texture.
    GLubyte *floor_texture = read_rgb_texture("../image_test/floor.rgb", &width, &height);

    std::cout << "floor texture " << width << ", " <<  height << "\n";

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &combined_texture_units);
    std::cout << "Limit 1 " <<  texture_units << " limit 2 " << combined_texture_units << std::endl;

    glGenTextures(1, &floor_texture_id);TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, floor_texture_id);TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, floor_texture);TEST_OPENGL_ERROR();
    glGenerateMipmap(GL_TEXTURE_2D);

    tex_location = glGetUniformLocation(program_id, "floor_sampler");TEST_OPENGL_ERROR();
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
        std::sprintf(filename, "../image_test/caustics_v2/CausticsRender_%03d.tga", i + 1);
        auto *caustic_texture = tifo::load_gray_image(filename, &width, &height)->get_buffer();
        std::cout << "texture id: " << caustics_id[i] << '\n';

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
    tex_location = glGetUniformLocation(program_id, "caustic_sampler");TEST_OPENGL_ERROR();
    glUniform1i(tex_location, 1);TEST_OPENGL_ERROR();

    unsigned char blue[3] = {0, 89, 179};

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

std::string load(const std::string &filename) {
    std::ifstream input_src_file(filename, std::ios::in);
    std::string ligne;
    std::string file_content="";
    if (input_src_file.fail()) {
        std::cerr << "FAIL\n";
        return "";
    }
    while(getline(input_src_file, ligne)) {
        file_content = file_content + ligne + "\n";
    }
    file_content += '\0';
    input_src_file.close();
    return file_content;
}

bool init_shaders() {
    std::string vertex_src = load("../src/vertex.vert");
    std::string fragment_src = load("../src/fragment.frag");
    GLuint shader_id[2];
    GLint compile_status = GL_TRUE;
    char *vertex_shd_src = (char*)std::malloc(vertex_src.length()*sizeof(char));
    char *fragment_shd_src = (char*)std::malloc(fragment_src.length()*sizeof(char));
    vertex_src.copy(vertex_shd_src,vertex_src.length());
    fragment_src.copy(fragment_shd_src,fragment_src.length());


    shader_id[0] = glCreateShader(GL_VERTEX_SHADER);TEST_OPENGL_ERROR();
    shader_id[1] = glCreateShader(GL_FRAGMENT_SHADER);TEST_OPENGL_ERROR();

    glShaderSource(shader_id[0], 1, (const GLchar**)&(vertex_shd_src), 0);TEST_OPENGL_ERROR();
    glShaderSource(shader_id[1], 1, (const GLchar**)&(fragment_shd_src), 0);TEST_OPENGL_ERROR();
    for(int i = 0 ; i < 2 ; i++) {
        glCompileShader(shader_id[i]);TEST_OPENGL_ERROR();
        glGetShaderiv(shader_id[i], GL_COMPILE_STATUS, &compile_status);
        if(compile_status != GL_TRUE) {
            GLint log_size;
            char *shader_log;
            glGetShaderiv(shader_id[i], GL_INFO_LOG_LENGTH, &log_size);
            shader_log = (char*)std::malloc(log_size+1); /* +1 pour le caractere de fin de chaine '\0' */
            if(shader_log != 0) {
                glGetShaderInfoLog(shader_id[i], log_size, &log_size, shader_log);
                std::cerr << "SHADER " << i << ": " << shader_log << std::endl;
                std::free(shader_log);
            }
            std::free(vertex_shd_src);
            std::free(fragment_shd_src);
            glDeleteShader(shader_id[0]);
            glDeleteShader(shader_id[1]);
            return false;
        }
    }
    std::free(vertex_shd_src);
    std::free(fragment_shd_src);


    GLint link_status=GL_TRUE;
    program_id=glCreateProgram();TEST_OPENGL_ERROR();
    if (program_id==0) return false;

    for(int i = 0 ; i < 2 ; i++) {
        glAttachShader(program_id, shader_id[i]);TEST_OPENGL_ERROR();
    }

    glLinkProgram(program_id);TEST_OPENGL_ERROR();
    glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);

    if (link_status!=GL_TRUE) {
        GLint log_size;
        char *program_log;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_size);
        program_log = (char*)std::malloc(log_size+1); /* +1 pour le caractere de fin de chaine '\0' */
        if(program_log != 0) {
            glGetProgramInfoLog(program_id, log_size, &log_size, program_log);
            std::cerr << "Program " << program_log << std::endl;
            std::free(program_log);
        }
        glDeleteProgram(program_id);TEST_OPENGL_ERROR();
        glDeleteShader(shader_id[0]);TEST_OPENGL_ERROR();
        glDeleteShader(shader_id[1]);TEST_OPENGL_ERROR();
        program_id=0;
        return false;
    }
    glUseProgram(program_id);TEST_OPENGL_ERROR();
    return true;
}

int main(int argc, char *argv[]) {
    init_glut(argc, argv);
    if (!init_glew())
        std::exit(1);

    init_GL();
    init_shaders();

    init_object_vbo();
    init_uniform();
    init_textures();

    glutMainLoop();
}