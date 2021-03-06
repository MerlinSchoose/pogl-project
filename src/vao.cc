//
// Created by simon on 7/15/21.
//

#include "vao.hh"

Vao *
Vao::make_vao(GLuint vertex_location, std::vector<GLfloat> vertices, GLuint texture_id, GLuint uv_location, std::vector<GLfloat> uv, std::vector<GLuint> indices) {
    auto vao = new Vao;
    glBindVertexArray(vao->id);TEST_OPENGL_ERROR();
    if (vertex_location != -1) vao->vbo_ids.push_back(-1);
    if (vertex_location != -1) vao->vbo_ids.push_back(-1);
    glGenBuffers(vao->vbo_ids.size(), vao->vbo_ids.data());TEST_OPENGL_ERROR();

    if (vertex_location != -1) {
        vao->draw_size = vertices.size();
        glBindBuffer(GL_ARRAY_BUFFER, vao->vbo_ids[0]);TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
        glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(vertex_location);TEST_OPENGL_ERROR();
    }

    if (uv_location != -1) {
        glBindBuffer(GL_ARRAY_BUFFER, vao->vbo_ids[1]);TEST_OPENGL_ERROR();
        glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(float), uv.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
        glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, nullptr);TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(uv_location);TEST_OPENGL_ERROR();
    }

    if (!indices.empty()) {
        vao->draw_size = indices.size();
        vao->using_indices = true;
        glGenBuffers(1, &vao->ebo_id);TEST_OPENGL_ERROR();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->ebo_id);TEST_OPENGL_ERROR();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);TEST_OPENGL_ERROR();
    }

    glBindVertexArray(0);TEST_OPENGL_ERROR();
    if (texture_id != -1) {
        vao->texture_id = texture_id;
    }

    return vao;
}

void Vao::draw() {
    glBindVertexArray(id);TEST_OPENGL_ERROR();
    if (texture_id != -1) {
        glActiveTexture(GL_TEXTURE0);TEST_OPENGL_ERROR();
        glBindTexture(GL_TEXTURE_2D, texture_id);TEST_OPENGL_ERROR();
    }
    if (using_indices) {
        glDrawElements(GL_TRIANGLES, draw_size, GL_UNSIGNED_INT, 0);TEST_OPENGL_ERROR();
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, draw_size);TEST_OPENGL_ERROR();
    }
    glBindVertexArray(0);TEST_OPENGL_ERROR();
}
