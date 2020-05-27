#include "Bsp.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Texture.h"
#include "ShaderProgram.h"
#include "LightmapNode.h"
#include "VertexBuffer.h"
#include "primitives.h"
#include "PointEntRenderer.h"

#define LIGHTMAP_ATLAS_SIZE 512

enum RenderFlags {
	RENDER_TEXTURES = 1,
	RENDER_LIGHTMAPS = 2,
	RENDER_WIREFRAME = 4,
	RENDER_ENTS = 8,
	RENDER_SPECIAL = 16,
	RENDER_SPECIAL_ENTS = 32,
	RENDER_POINT_ENTS = 64,
};

struct LightmapInfo {
	// each face can have 4 lightmaps, and those may be split across multiple atlases
	int atlasId[MAXLIGHTMAPS];
	int x[MAXLIGHTMAPS];
	int y[MAXLIGHTMAPS];

	int w, h;

	float midTexU, midTexV;
	float midPolyU, midPolyV;
};

struct RenderEnt {
	mat4x4 modelMat; // model matrix for rendering
	vec3 offset; // vertex transformations for picking
	int modelIdx;
	EntCube* pointEntCube;
};

struct RenderGroup {
	lightmapVert* wireframeVerts; // verts for rendering wireframe
	lightmapVert* verts;
	int vertCount;
	int wireframeVertCount;
	Texture* texture;
	Texture* lightmapAtlas[MAXLIGHTMAPS];
	VertexBuffer* buffer;
	VertexBuffer* wireframeBuffer;
	bool transparent;
};

struct RenderModel {
	RenderGroup* renderGroups;
	int groupCount;
};

struct FaceMath {
	mat4x4 worldToLocal; // transforms world coordiantes to this face's plane's coordinate system
	vec3 normal;
	float fdist;
	vec3* verts; // skips the edge lookups
	int vertCount;
};

struct PickInfo {
	int mapIdx;
	int entIdx;
	int modelIdx;
	int faceIdx;
	float bestDist;
	bool valid;
};

class BspRenderer {
public:
	Bsp* map;

	BspRenderer(Bsp* map, ShaderProgram* bspShader, ShaderProgram* colorShader, PointEntRenderer* fgd);
	~BspRenderer();

	void render(int highlightEnt);

	void drawModel(int modelIdx, bool transparent, bool highlight, bool edgesOnly);
	void drawPointEntities(int highlightEnt);

	bool pickPoly(vec3 start, vec3 dir, PickInfo& pickInfo);
	bool pickPoly(vec3 start, vec3 dir, vec3 offset, int modelIdx, PickInfo& pickInfo);

	void refreshEnt(int entIdx);
	int refreshModel(int modelIdx);
	void refreshFace(int faceIdx);

	// calculate vertex positions and uv coordinates once for faster rendering
	// also combines faces that share similar properties into a single buffer
	void preRenderFaces();
	void preRenderEnts();
	void calcFaceMaths();

	void updateLightmapInfos();

private:
	ShaderProgram* bspShader;
	ShaderProgram* colorShader;
	PointEntRenderer* pointEntRenderer;

	LightmapInfo* lightmaps;
	RenderEnt* renderEnts;
	RenderModel* renderModels;
	FaceMath* faceMaths;

	int numRenderModels;
	int numRenderLightmapInfos;
	int numFaceMaths;

	Texture** glTextures;
	Texture** glLightmapTextures;
	Texture* whiteTex;
	Texture* redTex;
	Texture* yellowTex;
	Texture* greyTex;
	Texture* blackTex;

	void loadTextures();
	void loadLightmaps();
};