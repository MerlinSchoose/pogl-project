//Vertex List
static const std::vector<GLfloat> floor_vbo {
        -5000.0, -120.0, 5000.0,
        5000.0, -120.0, 5000.0,
        -5000.0, -120.0, -5000.0,

        -5000.0, -120.0, -5000.0,
        5000.0, -120.0, 5000.0,
        5000.0, -120.0, -5000.0,
};

static const std::vector<GLfloat> surface_vbo {
        -5000.0, 0.0, 5000.0,
        5000.0, 0.0, 5000.0,
        -5000.0, 0.0, -5000.0,

        -5000.0, 0.0, -5000.0,
        5000.0, 0.0, 5000.0,
        5000.0, 0.0, -5000.0,
};

static const std::vector<GLfloat> background_vbo {
        -5000.0, -2500.0, 5000.0,
        5000.0, -2500.0, 5000.0,
        -5000.0, -2500.0, -5000.0,

        -5000.0, -2500.0, -5000.0,
        5000.0, -2500.0, 5000.0,
        5000.0, -2500.0, -5000.0,

        -5000.0, 2500.0, 5000.0,
        5000.0, 2500.0, 5000.0,
        -5000.0, 2500.0, -5000.0,

        -5000.0, 2500.0, -5000.0,
        5000.0, 2500.0, 5000.0,
        5000.0, 2500.0, -5000.0,

        -5000.0, -2500.0, 5000.0,
        5000.0, -2500.0, 5000.0,
        -5000.0, 2500.0, 5000.0,

        5000.0, -2500.0, 5000.0,
        5000.0, 2500.0, 5000.0,
        -5000.0, 2500.0, 5000.0,

        -5000.0, -2500.0, -5000.0,
        5000.0, -2500.0, -5000.0,
        -5000.0, 2500.0, -5000.0,

        5000.0, -2500.0, -5000.0,
        5000.0, 2500.0, -5000.0,
        -5000.0, 2500.0, -5000.0,

        -5000.0, -2500.0, -5000.0,
        -5000.0, -2500.0, 5000.0,
        -5000.0, 2500.0, -5000.0,

        -5000.0, -2500.0, 5000.0,
        -5000.0, 2500.0, 5000.0,
        -5000.0, 2500.0, -5000.0,

        5000.0, -2500.0, -5000.0,
        5000.0, -2500.0, 5000.0,
        5000.0, 2500.0, -5000.0,

        5000.0, -2500.0, 5000.0,
        5000.0, 2500.0, 5000.0,
        5000.0, 2500.0, -5000.0,
};

static const std::vector<GLfloat> uv_buffer_data
        { 0.0, 1.0,
          1.0, 1.0,
          0.0, 0.0,

          0.0, 0.0,
          1.0, 1.0,
          1.0, 0.0,
        };