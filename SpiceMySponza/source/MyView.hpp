#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>


class MyView : public tygra::WindowViewDelegate
{
public:
    
    MyView();
    
    ~MyView();
    
    void setScene(const sponza::Context * scene);

private:

    void windowViewWillStart(tygra::Window * window) override;
    
    void windowViewDidReset(tygra::Window * window,
                            int width,
                            int height) override;

    void windowViewDidStop(tygra::Window * window) override;
    
    void windowViewRender(tygra::Window * window) override;

private:

    const sponza::Context * scene_;

	// Me from here down
	GLuint shader_program_{ 0 };

	const static GLuint kNullId = 0;

	// DONE: define values for your Vertex attributes
	enum VertexAttribIndexes {
		kVertexPosition = 0,
		kVertexNormal = 1,
		kVertexColour = 2,
		kVertexTexture = 3
	};
	enum FragmentDataIndexes {
		kFragmentColour = 0
	};
	enum TextureIndexes {
		kDiffuseTexture = 0,
		kSpecularTexture = 1
	};

	// DONE: create a mesh structure to hold VBO ids etc.
	struct Mesh
	{
		sponza::MeshId id{ 0 };
		int elementCount{ 0 };

		// Element array
		GLuint element_vbo{ 0 };

		// Vertex position (combined) array
		GLuint position_vbo{ 0 };

		// Vertex array settings array
		GLuint vao{ 0 };
	};

	struct MaterialData
	{
		bool hasDiffuse{ false };
		bool hasSpecular{ false };
		GLuint diffuseId{ 0 };
		GLuint specularId{ 0 };
	};
	
	// DONE: create a container of these mesh e.g.
	std::vector<Mesh> meshBank;
	std::unordered_map<sponza::MaterialId, MaterialData> materialBank;

	void bindInterleaved(
		const sponza::MeshId& id,
		const std::vector<unsigned int>& elements,
		const std::vector<sponza::Vector3>& positions,
		const std::vector<sponza::Vector3>& normals,
		const std::vector<sponza::Vector2>& textures);

	GLuint createTexture(std::string imageLocation);
};
