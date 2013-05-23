


#ifdef WIN32
	#include<Windows.h>
#endif

#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <stdio.h>

#ifdef TARGET_GLES2
  #include <EGL/egl.h>
  #include <GLES2/gl2.h>
#else
  #define GL_GLEXT_PROTOTYPES
  #include <GL/gl.h>
  #include <GL/glext.h>
#endif

#include<rh_window.h>
#include<rh_texture_loader.h>

#define LOGI(...) ((void)printf(__VA_ARGS__))
#define LOGW(...) ((void)printf(__VA_ARGS__))

static void __report_gl_err(const char * file, const char * func, int line) {

	GLenum e;

	while( (e = glGetError()) != GL_NO_ERROR ) {

		printf("RH %s:%s:%d gl error %d\n", file, func, line, e);
	}
}
#define GL_ERROR() __report_gl_err(__FILE__,__FUNCTION__,__LINE__)

static GLuint compile_shader( GLenum shaderType, const GLchar * src, int src_len ) {

	GLuint shader = glCreateShader( shaderType );

	glShaderSource( shader, 1, &src, &src_len );
	glCompileShader(shader);

	GLint param = GL_FALSE;

	glGetShaderiv(shader,GL_COMPILE_STATUS, &param);

	if( param != GL_TRUE ) {

		GLint logLength = 0;
		GLint rlogLength = 0;
		GLchar * log = NULL;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		log = (GLchar*)alloca( logLength+1 );
		if(log) {
			memset(log,0,logLength+1);
			glGetShaderInfoLog(shader, logLength, &rlogLength, log);
			LOGI("RH %s",log);
		}

		glDeleteShader(shader);
		shader = 0;
	}

	return shader;
}

static GLint link_program(GLuint program, GLuint vshader, GLuint fshader) {

	GLint linkparam = GL_FALSE;
	GLint validparam = GL_FALSE;

	glAttachShader( program, vshader );
	glAttachShader( program, fshader );
	glLinkProgram( program );
	glGetProgramiv( program, GL_LINK_STATUS, &linkparam );
	glValidateProgram( program );
	glGetProgramiv( program, GL_LINK_STATUS, &validparam );

	if(linkparam == GL_TRUE && validparam == GL_TRUE)
		return GL_TRUE;

	return GL_FALSE;
}

static GLuint create_simple_program(const GLchar * vsource, int vsource_size, const GLchar *fsource, int fsource_size ) {


	GLuint program = 0;
	GLuint vshader = 0;
	GLuint fshader = 0;
	GLint  linkedparam = GL_FALSE;

	program = glCreateProgram();
	vshader = compile_shader( GL_VERTEX_SHADER,   vsource, vsource_size );
	fshader = compile_shader( GL_FRAGMENT_SHADER, fsource, fsource_size );

	if(program && vshader && fshader)
		linkedparam = link_program( program, vshader, fshader);

	if( vshader )
		glDeleteShader( vshader );
	if( fshader )
		glDeleteShader( fshader );

	if(program && (linkedparam != GL_TRUE))
		glDeleteProgram(program);

	if (linkedparam != GL_TRUE)
		program = 0;

	return program;
}

static GLuint _create_2d_program( ) {

	static const GLchar vertex_shader_src[] =
		"attribute vec2 positionAttr;\n"
		"attribute vec2 texcoordAttr;\n"
		"\n"
		"varying vec2 texcoordVar;\n"
		"\n"
		"void main(void)\n"
		"{\n"
		"	gl_Position = vec4(positionAttr.x,positionAttr.y,0.0,1.0);\n"
		"	texcoordVar = texcoordAttr;\n"
		"}\n";


	static const GLchar fragment_shader_src[] =
	//	"precision mediump float;\n"
		"varying vec2 texcoordVar;\n"
		"uniform sampler2D texSamp;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = texture2D(texSamp, texcoordVar);\n"
		"}\n";

	return create_simple_program(
			vertex_shader_src,
			sizeof vertex_shader_src,
			fragment_shader_src,
			sizeof fragment_shader_src
	);
}

static GLuint _create_2d_array_program( ) {

	static const GLchar vertex_shader_src[] =
		"attribute vec2 positionAttr;\n"
		"attribute vec3 texcoordAttr;\n"
		"\n"
		"varying vec3 texcoordVar;\n"
		"\n"
		"void main(void)\n"
		"{\n"
		"	gl_Position = vec4(positionAttr.x,positionAttr.y,0.0,1.0);\n"
		"	texcoordVar = texcoordAttr;\n"
		"}\n";


	static const GLchar fragment_shader_src[] =
	//	"precision mediump float;\n"
		"#extension GL_EXT_texture_array : enable\n"
		"varying vec3 texcoordVar;\n"
		"uniform sampler2DArray texSamp;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = texture2DArray(texSamp, texcoordVar);\n"
		"}\n";

	return create_simple_program(
			vertex_shader_src,
			sizeof vertex_shader_src,
			fragment_shader_src,
			sizeof fragment_shader_src
	);
}

static GLuint create_program(GLenum target) {
 
  GLuint prog;
  GLint texLoc;
  
  if(target == GL_TEXTURE_2D_ARRAY_EXT)
    prog = _create_2d_array_program();
  else
    prog = _create_2d_program();
  
  texLoc = glGetUniformLocation(prog, "texSamp");

  glUseProgram( prog );
  
  glUniform1i( texLoc , 0);
  
  return prog;	
}

GLuint create_vbuffer(GLuint prog, GLenum target, rh_texpak_handle pak, rh_texpak_idx idx) {
  
  GLuint vbuff = 0;
  GLuint pos_attr_loc;
  GLuint tex_attr_loc;
  
  glGenBuffers( 1, &vbuff );
  glBindBuffer(GL_ARRAY_BUFFER, vbuff);
  
  pos_attr_loc = glGetAttribLocation(prog, "positionAttr");
  tex_attr_loc = glGetAttribLocation(prog, "texcoordAttr");
  
  if( target == GL_TEXTURE_2D_ARRAY_EXT ) {
    
    GLfloat vbuff_data[] = {
    //    x     y     s     t     q
      -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, // tl
      -1.0f,-1.0f, 0.0f, 1.0f, 0.0f, // bl
       1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // tr
       1.0f,-1.0f, 1.0f, 1.0f, 0.0f, // br
    };
    
    GLfloat texcoords[12];
    
    rh_texpak_get_coords3d(pak,idx,texcoords);
    
    vbuff_data[ 2] = texcoords[ 0];
    vbuff_data[ 3] = texcoords[ 1];
    vbuff_data[ 4] = texcoords[ 2];
    vbuff_data[ 7] = texcoords[ 3];
    vbuff_data[ 8] = texcoords[ 4];
    vbuff_data[ 9] = texcoords[ 5];
    vbuff_data[12] = texcoords[ 6];
    vbuff_data[13] = texcoords[ 7];
    vbuff_data[14] = texcoords[ 8];
    vbuff_data[17] = texcoords[ 9];
    vbuff_data[18] = texcoords[10];
    vbuff_data[19] = texcoords[11];
    
    {
      printf(	"{%8f,%8f,%8f,%8f,%8f}\n"
		"{%8f,%8f,%8f,%8f,%8f}\n"
		"{%8f,%8f,%8f,%8f,%8f}\n"
		"{%8f,%8f,%8f,%8f,%8f}\n",
		    vbuff_data[ 0],vbuff_data[ 1],vbuff_data[ 2],vbuff_data[ 3],vbuff_data[ 4],
		    vbuff_data[ 5],vbuff_data[ 6],vbuff_data[ 7],vbuff_data[ 8],vbuff_data[ 9],
		    vbuff_data[10],vbuff_data[11],vbuff_data[12],vbuff_data[13],vbuff_data[14],
		    vbuff_data[15],vbuff_data[16],vbuff_data[17],vbuff_data[18],vbuff_data[19]);
    }
  
    glBufferData(GL_ARRAY_BUFFER, sizeof vbuff_data, vbuff_data, GL_STATIC_DRAW);
    glVertexAttribPointer(pos_attr_loc,2,GL_FLOAT,GL_FALSE,5 * sizeof(GLfloat),(const void*)(0 * sizeof(GLfloat)));
    glVertexAttribPointer(tex_attr_loc,3,GL_FLOAT,GL_FALSE,5 * sizeof(GLfloat),(const void*)(2 * sizeof(GLfloat)));
    
  } else {
    
    GLfloat vbuff_data[] = {
    //    x     y     s     t
      -1.0f, 1.0f, 0.0f, 0.0f, // tl
      -1.0f,-1.0f, 0.0f, 1.0f, // bl
       1.0f, 1.0f, 1.0f, 0.0f, // tr
       1.0f,-1.0f, 1.0f, 1.0f, // br
    };
    
    GLfloat texcoords[8];
    
    rh_texpak_get_coords2d(pak,idx,texcoords);
    
    vbuff_data[ 2] = texcoords[ 0];
    vbuff_data[ 3] = texcoords[ 1];
    vbuff_data[ 6] = texcoords[ 2];
    vbuff_data[ 7] = texcoords[ 3];
    vbuff_data[10] = texcoords[ 4];
    vbuff_data[11] = texcoords[ 5];
    vbuff_data[14] = texcoords[ 6];
    vbuff_data[15] = texcoords[ 7];
    
    {
      printf(	"{%8f,%8f,%8f,%8f}\n"
		"{%8f,%8f,%8f,%8f}\n"
		"{%8f,%8f,%8f,%8f}\n"
		"{%8f,%8f,%8f,%8f}\n",
		    vbuff_data[ 0],vbuff_data[ 1],vbuff_data[ 2],vbuff_data[ 3],
		    vbuff_data[ 4],vbuff_data[ 5],vbuff_data[ 6],vbuff_data[ 7],
		    vbuff_data[ 8],vbuff_data[ 9],vbuff_data[10],vbuff_data[11],
		    vbuff_data[12],vbuff_data[13],vbuff_data[14],vbuff_data[15]);
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof vbuff_data, vbuff_data, GL_STATIC_DRAW);
    glVertexAttribPointer(pos_attr_loc,2,GL_FLOAT,GL_FALSE,4 * sizeof(GLfloat),(const void*)(0 * sizeof(GLfloat)));
    glVertexAttribPointer(tex_attr_loc,2,GL_FLOAT,GL_FALSE,4 * sizeof(GLfloat),(const void*)(2 * sizeof(GLfloat)));
  }
 
  glEnableVertexAttribArray(pos_attr_loc); 
  glEnableVertexAttribArray(tex_attr_loc);
  
  return vbuff;
}

int main(int argc, char **argv) {
    
  int exitflag = 0;
  
  rh_display_handle display = 0;
  rh_screen_handle  screen = 0;
  rh_window_handle  window = 0;
  rh_render_handle  render = 0;
  rh_input_handle   input = 0;
  rh_input_data     input_data = 0;
  
  rh_texpak_handle texpak = 0;
  rh_texpak_idx	   texidx = 0;
  
  GLuint program = 0;
  GLenum target;
  GLuint vbuffer;
  GLuint texture;
  
  if(argc != 3) {
    printf("useage: %s \"texture pak file\" \"texture name\"\n", argv[0]);
    exit(1);
  }
  
  
  
  rh_display_create(&display);
  rh_screen_create_default(&screen, display);
  
  {
//   rh_window_attr_t  window_attr = 0;
//    int window_width;
//    int window_height;
    
//    rh_texpak_get_size(texpak, texidx, &window_width, &window_height);   
//    rh_window_attr_create(&window_attr);
//    rh_window_attr_seti(window_attr, "w", window_width);
//    rh_window_attr_seti(window_attr, "h", window_height);
//    rh_window_create(&window, window_attr, screen);
      rh_window_create(&window, NULL, screen);
//    rh_window_attr_destroy(window_attr);
  }
 
  rh_render_create(&render,window, 2,1,0);
  rh_bind_render_window(render, window);
  rh_input_create(&input, window);
  
  if( rh_texpak_open(argv[1], &texpak) != 0 ) {
    printf("cant open %s\n", argv[1]);
    exit(1);
  }
  
  if( rh_texpak_lookup(texpak, argv[2], &texidx) != 0) {
    printf("cant find %s in %s\n", argv[2], argv[1]);
    exit(1);
  }
    
  rh_texpak_get_textarget(texpak, &target);
  program = create_program(target);
  
  vbuffer = create_vbuffer( program, target, texpak, texidx);
  
  {
    
    rh_texpak_get_texture(texpak, texidx, &texture);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, texture);
  }
  
//glClearColor(1.0f,0.0f,1.0f,1.0f);
  
  GL_ERROR();
  
  while(!exitflag) {
  
//  glClear(GL_COLOR_BUFFER_BIT);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    GL_ERROR();
    
    rh_window_swapbuffers(window);
    
    if(( input_data = rh_input_get( input ) )) {
    
      switch( rh_input_gettype( input_data ) ) {

	case RH_INPUT_KEYPRESS:
	{
	  rh_input_key_enum_t k;
	  rh_input_getkey(input_data, &k);
	  if( k == RH_INPUT_KEY_ESCAPE)
		exitflag = 1;
	  break;
	}
      }
    }
  }
  
  rh_input_destroy(input);
  rh_bind_render_window(render, NULL);
  rh_render_destroy(render);
  rh_window_destroy(window);
  rh_screen_destroy(screen);
  rh_display_destroy(display);
  
  return 0;
}

