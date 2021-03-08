#include "MyView.hpp"
#include <sponza/sponza.hpp>
#include <tygra/FileHelper.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>

//#include <cassert>

MyView::MyView()
{
}

MyView::~MyView() {
}

void MyView::setScene(const sponza::Context * scene)
{
    scene_ = scene;
}

void MyView::windowViewWillStart(tygra::Window * window)
{
    assert(scene_ != nullptr);

	GLint compile_status = GL_FALSE;

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertex_shader_string
		= tygra::createStringFromFile("resource:///sponza_vs.glsl");
	const char * vertex_shader_code = vertex_shader_string.c_str();
	glShaderSource(vertex_shader, 1,
		(const GLchar **)&vertex_shader_code, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(vertex_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragment_shader_string
		= tygra::createStringFromFile("resource:///sponza_fs.glsl");
	const char * fragment_shader_code = fragment_shader_string.c_str();
	glShaderSource(fragment_shader, 1,
		(const GLchar **)&fragment_shader_code, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(fragment_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	// Create shader program & shader in variables
	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertex_shader);

	// DONE: glBindAttribLocation for all shader streamed IN variables e.g.
	glBindAttribLocation(shader_program_, kVertexPosition, "vertex_position");
	glBindAttribLocation(shader_program_, kVertexNormal, "vertex_normal");
	glBindAttribLocation(shader_program_, kVertexTexture, "vertex_texture");

	glDeleteShader(vertex_shader);
	glAttachShader(shader_program_, fragment_shader);
	glBindFragDataLocation(shader_program_, kFragmentColour, "fragment_colour");
	glDeleteShader(fragment_shader);
	glLinkProgram(shader_program_);

	GLint link_status = GL_FALSE;
	glGetProgramiv(shader_program_, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(shader_program_, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	/*
		The framework provides a builder class that allows access to all the mesh data	
	*/
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	sponza::GeometryBuilder builder;
	const auto& source_meshes = builder.getAllMeshes();

	// We can loop through each mesh in the scene
	for (auto& source : source_meshes)
	{
		// Each mesh has an id that you will need to remember for later use
		const auto& id = source.getId();
		// DONE: you also need to get the normals, elements and texture coordinates in a similar way
		const auto& ele = source.getElementArray();
		const auto& pos = source.getPositionArray();
		const auto& nor = source.getNormalArray();
		const auto& tex = source.getTextureCoordinateArray();
		// DONE: Create VBOs for position, normals, elements and texture coordinates
		// DONE: Create a VAO to wrap all the VBOs
		// DONE: store in a mesh structure and add to a container for later use
		bindInterleaved(id, ele, pos, nor, tex);
	}

	std::vector<GLuint> texIdBank;
	std::vector<std::string> texNameBank;
	const auto& materials = scene_->getAllMaterials();
	for (auto& material : materials)
	{
		MaterialData matData;

		if (material.getDiffuseTexture() != "")
		{
			matData.hasDiffuse = true;
			auto tempName = std::find(texNameBank.begin(), 
				texNameBank.end(), material.getDiffuseTexture());

			if (tempName == texNameBank.end())
			{
				matData.diffuseId = createTexture(material.getDiffuseTexture());

				texIdBank.push_back(matData.diffuseId);
				texNameBank.push_back(material.getDiffuseTexture());
			}
			else
			{
				matData.diffuseId = texIdBank[tempName - texNameBank.begin()];
			}
		}

		if (material.getSpecularTexture() != "")
		{
			matData.hasSpecular = true;
			auto tempName = std::find(texNameBank.begin(),
				texNameBank.end(), material.getSpecularTexture());

			if (tempName == texNameBank.end())
			{
				matData.specularId = createTexture(material.getSpecularTexture());

				texIdBank.push_back(matData.specularId);
				texNameBank.push_back(material.getSpecularTexture());
			}
			else
			{
				matData.specularId = texIdBank[tempName - texNameBank.begin()];
			}
		}
		materialBank[material.getId()] = matData;
	}
}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
    glViewport(0, 0, width, height);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
	for (auto& mat : materialBank)
	{
		if (mat.second.hasDiffuse)
			glDeleteTextures(1, &mat.second.diffuseId);

		if (mat.second.hasSpecular)
			glDeleteTextures(1, &mat.second.specularId);
	}

	for (auto& mesh : meshBank)
	{
		glDeleteBuffers(1, &mesh.position_vbo);
		glDeleteBuffers(1, &mesh.element_vbo);
		glDeleteVertexArrays(1, &mesh.vao);
	}

	glDeleteProgram(shader_program_);
}

void MyView::windowViewRender(tygra::Window * window)
{
	assert(scene_ != nullptr);

	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Clear buffers from previous frame
	glClearColor(0.f, 0.f, 0.25f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_program_);
	 
	// Compute viewport
	GLint viewport_size[4];
	glGetIntegerv(GL_VIEWPORT, viewport_size);
	const float aspect_ratio = viewport_size[2] / (float)viewport_size[3];

	// Note: the code above is supplied for you and already works

	// DONE: Compute projection matrix
	const float& FoVDeg = scene_->getCamera().getVerticalFieldOfViewInDegrees();
	const float& nearPlaneDist = scene_->getCamera().getNearPlaneDistance();
	const float& farPlaneDist = scene_->getCamera().getFarPlaneDistance();
	glm::mat4 projection_xform = glm::perspective(
		glm::radians(FoVDeg), 
		aspect_ratio, 
		nearPlaneDist, 
		farPlaneDist);

	GLuint projection_xform_id = glGetUniformLocation(shader_program_, "projection_xform");
	glUniformMatrix4fv(projection_xform_id, 1, GL_FALSE, glm::value_ptr(projection_xform));

	// DONE: Compute view matrix
	// You can get the camera position, look at and world up from the scene e.g.
	const glm::vec3& cameraPos = (const glm::vec3&)scene_->getCamera().getPosition();
	const glm::vec3& cameraLook = (const glm::vec3&)scene_->getCamera().getDirection();
	const glm::vec3& upDir = (const glm::vec3&)scene_->getUpDirection();
	glm::mat4 view_xform = glm::lookAt(cameraPos, cameraPos + cameraLook, upDir);

	GLuint view_xform_id = glGetUniformLocation(shader_program_, "view_xform");
	glUniformMatrix4fv(view_xform_id, 1, GL_FALSE, glm::value_ptr(view_xform));

	GLuint camera_pos_id = glGetUniformLocation(shader_program_, "camera_position");
	glUniform3fv(camera_pos_id, 1, glm::value_ptr(cameraPos));

	// DONE: create combined view * projection matrix and pass to shader as a uniform
	glm::mat4 proj_view_xform = projection_xform * view_xform;

	GLuint proj_view_xform_id = glGetUniformLocation(shader_program_, "combined_xform");
	glUniformMatrix4fv(proj_view_xform_id, 1, GL_FALSE, glm::value_ptr(proj_view_xform));

	// DONE: Get light data from scene via scene_->getAllLights()
	// then plug the values into the shader
	const auto& lightBank = scene_->getAllLights();
	{
		int numLights = (int)lightBank.size();
		GLuint num_light_id = glGetUniformLocation(shader_program_, "numLights");
		glUniform1i(num_light_id, numLights);

		for (int i = 0; i < numLights; i++)
		{
			GLuint light_position_id = glGetUniformLocation(
				shader_program_,
				("light_sources[" + std::to_string(i) + "].position").c_str());
			glUniform3fv(
				light_position_id,
				1,
				glm::value_ptr((const glm::vec3&)lightBank[i].getPosition()));

			GLuint light_range_id = glGetUniformLocation(
				shader_program_,
				("light_sources[" + std::to_string(i) + "].range").c_str());
			glUniform1f(light_range_id, (float)lightBank[i].getRange());

			GLuint light_intensity_id = glGetUniformLocation(
				shader_program_,
				("light_sources[" + std::to_string(i) + "].intensity").c_str());
			glUniform3fv(
				light_intensity_id,
				1,
				glm::value_ptr((const glm::vec3&)lightBank[i].getIntensity()));
		}
	}

	GLuint ambient_light_id = glGetUniformLocation(shader_program_, "ambient_light_intensity");
	glUniform3fv(ambient_light_id, 1, 
		glm::value_ptr((const glm::vec3&)scene_->getAmbientLightIntensity()));

	// DONE: Render each mesh
	for (auto& mesh : meshBank)
	{
		glBindVertexArray(mesh.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.element_vbo);

		// Each mesh can be repeated in the scene so we need to ask the scene 
		//for all instances of the mesh
		// and render each instance with its own model matrix
		// To get the instances we need to use the meshId we stored earlier e.g.
		const std::vector<unsigned int>& instances = scene_->getInstancesByMeshId(mesh.id);
		// then loop through all instances
		// for each instance you can call getTransformationMatrix 
		// this then needs passing to the shader as a uniform
		GLuint trans_xform_id = glGetUniformLocation(shader_program_, "trans_xform");
		GLuint combined_xform_id = glGetUniformLocation(shader_program_, "combined_xform");
		glm::mat4 combined_xform;
		const glm::vec3 nullVector = glm::vec3(0.0, 0.0, 0.0);
		GLuint has_diffuse_id = glGetUniformLocation(shader_program_, "hasDiffuse");
		GLuint has_specular_id = glGetUniformLocation(shader_program_, "hasSpecular");
		GLuint diffuse_texture_id = glGetUniformLocation(shader_program_, "diffuse_texture");
		GLuint specular_texture_id = glGetUniformLocation(shader_program_, "specular_texture");
		GLuint ambient_id = glGetUniformLocation(shader_program_, "ambient_colour");
		GLuint diffuse_id = glGetUniformLocation(shader_program_, "diffuse_colour");
		GLuint specular_id = glGetUniformLocation(shader_program_, "specular_colour");
		GLuint shininess_id = glGetUniformLocation(shader_program_, "material_shininess");
		GLuint is_shiny_id = glGetUniformLocation(shader_program_, "isShiny");

		for (unsigned int i : instances)
		{
			const glm::mat4 trans_xform =
				(const glm::mat4x3&)scene_->getInstanceById(i).getTransformationMatrix();

			glUniformMatrix4fv(trans_xform_id, 1, GL_FALSE, glm::value_ptr(trans_xform));

			combined_xform = proj_view_xform * trans_xform;
			glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE,
				glm::value_ptr(combined_xform));

			// TODO:
			// Materials - leave this until you get the main scene showing
			// Each instance of the mesh has its own material accessed like so:
			// Get material for this instance
			const auto& material_id = scene_->getInstanceById(i).getMaterialId();
			const auto& material = scene_->getMaterialById(material_id);

			glUniform1i(has_diffuse_id, materialBank[material_id].hasDiffuse);
			
			if (materialBank[material_id].hasDiffuse)
			{
				glActiveTexture(GL_TEXTURE0 + kDiffuseTexture);
				glBindTexture(GL_TEXTURE_2D,
					materialBank[material_id].diffuseId);
				glUniform1i(diffuse_texture_id, kDiffuseTexture);
			}
			glUniform3fv(ambient_id, 1,
				glm::value_ptr((const glm::vec3&)material.getAmbientColour()));

			glUniform3fv(diffuse_id, 1,
				glm::value_ptr((const glm::vec3&)material.getDiffuseColour()));

			glUniform1i(has_specular_id, materialBank[material_id].hasSpecular);

			if (materialBank[material_id].hasSpecular)
			{
				glActiveTexture(GL_TEXTURE0 + kSpecularTexture);
				glBindTexture(GL_TEXTURE_2D,
					materialBank[material_id].specularId);
				glUniform1i(specular_texture_id, kSpecularTexture);
			}

			glUniform3fv(specular_id, 1,
				glm::value_ptr((const glm::vec3&)material.getSpecularColour()));


			glUniform1f(shininess_id, material.getShininess());

			glUniform1i(is_shiny_id, material.isShiny());

			glDrawElements(GL_TRIANGLES, mesh.elementCount, GL_UNSIGNED_INT, 0);
		}
	}
}

struct Interleaved
{
	sponza::Vector3 pos;
	sponza::Vector3 nor;
	sponza::Vector2 tex;
};

void MyView::bindInterleaved(
	const sponza::MeshId& id,
	const std::vector<unsigned int>& elements, 
	const std::vector<sponza::Vector3>& positions,
	const std::vector<sponza::Vector3>& normals,
	const std::vector<sponza::Vector2>& textures)
{
	Mesh mesh;
	mesh.id = id;

	std::vector<Interleaved> omni;
	omni.reserve(positions.size());

	{
		Interleaved temp;
		for (int i = 0; i < positions.size(); i++)
		{
			temp.pos = positions[i];
			temp.nor = normals[i];
			temp.tex = textures[i];
			omni.push_back(temp);
		}
	}

	//Filling the element buffer with the element data
	glGenBuffers(1, &mesh.element_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.element_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		elements.size() * sizeof(unsigned int),
		elements.data(),
		GL_STATIC_DRAW);

	mesh.elementCount = (int)elements.size();

	glGenBuffers(1, &mesh.position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.position_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		omni.size() * sizeof(Interleaved), // size of data in bytes
		omni.data(), // pointer to the data
		GL_STATIC_DRAW);

	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.element_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.position_vbo);
	//Point the vertex shader to the right values in the omni vector
	glEnableVertexAttribArray(kVertexPosition);
	glVertexAttribPointer(kVertexPosition, 3, GL_FLOAT, GL_FALSE, 
		sizeof(Interleaved), (void*)offsetof(Interleaved, pos));
	glEnableVertexAttribArray(kVertexNormal);
	glVertexAttribPointer(kVertexNormal, 3, GL_FLOAT, GL_FALSE, 
		sizeof(Interleaved), (void*)offsetof(Interleaved, nor));
	glEnableVertexAttribArray(kVertexTexture);
	glVertexAttribPointer(kVertexTexture, 2, GL_FLOAT, GL_FALSE, 
		sizeof(Interleaved), (void*)offsetof(Interleaved, tex));

	glBindBuffer(GL_ARRAY_BUFFER, kNullId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kNullId);
	glBindVertexArray(kNullId);

	meshBank.push_back(mesh);
}

/*
 * Tutorial: Create a texture object from pixel data read from an PNG.
 *           There is no image loading in OpenGL, so we use a helper.
 */

GLuint MyView::createTexture(std::string imageLocation)
{
	GLuint id{ 0 };

	tygra::Image texture
		= tygra::createImageFromPngFile("resource:///" + imageLocation);
	if (texture.doesContainData())
	{
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width(), texture.height(), 0, pixel_formats[texture.componentsPerPixel()],
			GL_UNSIGNED_BYTE, texture.pixelData());
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glBindTexture(GL_TEXTURE_2D, kNullId);
	}
	return id;
}