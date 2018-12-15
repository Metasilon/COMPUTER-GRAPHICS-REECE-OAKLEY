#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "SDL2-2.0.8\include\SDL.h"
#include "SDL2-2.0.8\include\SDL_image.h"
#include "SDL2-2.0.8\include\SDL_mixer.h"

//load libraries
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2main.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2_image.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2_mixer.lib")

#pragma comment(linker,"/subsystem:console")

#include <iostream>
#include <fstream>
#include <assert.h>
#include <time.h>
#include "Table_File_core.h"
using namespace std;

struct Pos
{
	float x;
	float y;
};

struct Size
{
	float w, h;
};

struct Speed
{
	float x, y;
};

struct Vec2D
{
	float x;
	float y;
};

struct RGB
{
	unsigned char r, g, b;
};

namespace Tileset
{
	struct Tileset
	{
		int tile_w;
		int tile_h;
		int n_cols;
	};

	int get_Col(int tile_id, Tileset *t)
	{
		return t->tile_w * (tile_id % t->n_cols);
	}

	int get_Row(int tile_id, Tileset *t)
	{
		return t->tile_w * (tile_id / t->n_cols);
	}
}

namespace Collision
{
	enum { NO_COLLISION = 0, TOP_OF_1, RIGHT_OF_1, BOTTOM_OF_1, LEFT_OF_1 };

	int minkowski(float x0, float y0, float w0, float h0, float x1, float y1, float w1, float h1)
	{
		float w = 0.5 * (w0 + w1);
		float h = 0.5 * (h0 + h1);
		float dx = x0 - x1 + 0.5*(w0 - w1);
		float dy = y0 - y1 + 0.5*(h0 - h1);

		if (dx*dx <= w * w && dy*dy <= h * h)
		{
			float wy = w * dy;
			float hx = h * dx;

			if (wy > hx)
			{
				return (wy + hx > 0) ? BOTTOM_OF_1 : LEFT_OF_1;
			}
			else
			{
				return (wy + hx > 0) ? RIGHT_OF_1 : TOP_OF_1;
			}
		}
		return NO_COLLISION;
	}
}

namespace Particle_Emitter
{
	struct Particle_Emitter
	{
		Vec2D *pos;
		Vec2D *force;
		Vec2D *vel;
		float *life;
		int *state;
		int n_particles;

		float emitter_mass;
		float particle_mass;
		Vec2D emitter_pos;
		Vec2D emitter_force;
		Vec2D emitter_vel;

		RGB particle_color;
		float particle_size;
	};

	void init(Particle_Emitter *p, int n_particles)
	{
		p->n_particles = n_particles;

		//allocations
		p->pos = new Vec2D[p->n_particles];
		p->force = new Vec2D[p->n_particles];
		p->vel = new Vec2D[p->n_particles];
		p->life = new float[p->n_particles];
		p->state = new int[p->n_particles];

		memset(p->pos, 0, sizeof(Vec2D)*p->n_particles);
		memset(p->force, 0, sizeof(Vec2D)*p->n_particles);
		memset(p->vel, 0, sizeof(Vec2D)*p->n_particles);
		memset(p->life, 0, sizeof(float)*p->n_particles);
		memset(p->state, 0, sizeof(int)*p->n_particles);

		p->emitter_force = {};
		p->emitter_vel = {};
		p->emitter_pos = { 50, 50 };
		p->particle_color = { 165, 75, 0 };
		p->particle_size = 4.0;
		p->emitter_mass = 1.0;
		p->particle_mass = 0.1;
	}

	void spawn_Many(Particle_Emitter *p, Vec2D influence_min, Vec2D influence_max, int how_many, float min_life, float max_life)
	{
		for (int i = 0; i < p->n_particles; i++)
		{
			if (p->state[i] == 0)
			{
				if (--how_many < 0) break;

				p->state[i] = 1;

				p->pos[i] = p->emitter_pos;
				p->vel[i] = p->emitter_vel;
				p->life[i] = min_life + (max_life - min_life)*rand() / RAND_MAX;
				p->force[i] = {};

				p->force[i].x += influence_min.x + (influence_max.x - influence_min.x)*rand() / RAND_MAX;
				p->force[i].y += influence_min.y + (influence_max.y - influence_min.y)*rand() / RAND_MAX;
			}
		}
	}

	void clear_Forces_from_Particles(Particle_Emitter *p)
	{
		for (int i = 0; i < p->n_particles; i++)
		{
			if (p->state[i] != 0) p->force[i] = {};
		}
	}

	void add_Force_to_Particles(Particle_Emitter *p, Vec2D f)
	{
		for (int i = 0; i < p->n_particles; i++)
		{
			if (p->state[i] != 0)
			{
				p->force[i].x += f.x;
				p->force[i].y += f.y;
			}
		}
	}

	void update(Particle_Emitter *p, float time_elapsed)
	{
		//implicit euler, mass=1.0
		Vec2D accel;
		accel.x = p->emitter_force.x / p->emitter_mass;
		accel.y = p->emitter_force.y / p->emitter_mass;

		p->emitter_vel.x += accel.x*time_elapsed;
		p->emitter_vel.y += accel.y*time_elapsed;
		p->emitter_pos.x += p->emitter_vel.x*time_elapsed;
		p->emitter_pos.y += p->emitter_vel.y*time_elapsed;

		for (int i = 0; i < p->n_particles; i++)
		{
			if (p->state[i] == 0) continue;

			p->life[i] -= time_elapsed;
			if (p->life[i] <= 0.0)
			{
				p->state[i] = 0;
				continue;
			}

			Vec2D accel;
			accel.x = p->force[i].x / p->particle_mass;
			accel.y = p->force[i].y / p->particle_mass;
			p->vel[i].x += accel.x*time_elapsed;
			p->vel[i].y += accel.y*time_elapsed;
			p->pos[i].x += p->vel[i].x*time_elapsed;
			p->pos[i].y += p->vel[i].y*time_elapsed;
		}
	}

	void draw(Particle_Emitter *p, SDL_Renderer *renderer)
	{
		SDL_SetRenderDrawColor(renderer, p->particle_color.r, p->particle_color.g, p->particle_color.b, 255);
		for (int i = 0; i < p->n_particles; i++)
		{
			if (p->state[i] == 0) continue;

			SDL_Rect rect = { p->pos[i].x,p->pos[i].y,p->particle_size, p->particle_size };
			SDL_RenderFillRect(renderer, &rect);
		}
	}

}
	
	int main(int argc, char **argv)
	{
		//particle Emitters
		Vec2D t_min = { 0.0, -0.001 };
		Vec2D t_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter torch;
		Particle_Emitter::init(&torch, 150);
		torch.emitter_pos.x = 110;
		torch.emitter_pos.y = 327;
		torch.particle_color = { 255, 150, 0 };
		torch.particle_size = 6;

		Vec2D t2_min = { 0.0, -0.001 };
		Vec2D t2_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter torch2;
		Particle_Emitter::init(&torch2, 150);
		torch2.emitter_pos.x = 909;
		torch2.emitter_pos.y = 327;
		torch2.particle_color = { 255, 150, 0 };
		torch2.particle_size = 6;

		Vec2D s_min = { 0.0, -0.0001 };
		Vec2D s_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter sparkler;
		Particle_Emitter::init(&sparkler, 150);
		sparkler.emitter_pos.x = 265;
		sparkler.emitter_pos.y = 380;
		sparkler.particle_color = { 255, 255, 0 };
		sparkler.particle_size = 6;

		Vec2D s2_min = { 0.0, -0.0001 };
		Vec2D s2_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter sparkler2;
		Particle_Emitter::init(&sparkler2, 150);
		sparkler2.emitter_pos.x = 750;
		sparkler2.emitter_pos.y = 380;
		sparkler2.particle_color = { 255, 255, 0 };
		sparkler2.particle_size = 6;

		Vec2D fr_min = { -0.0, -0.001 };
		Vec2D fr_max = { -0.01, -0.0005 };
		Particle_Emitter::Particle_Emitter fog_r;
		Particle_Emitter::init(&fog_r, 1500);
		fog_r.emitter_pos.x = 1024;
		fog_r.emitter_pos.y = 512;
		fog_r.particle_color = { 167, 166, 157 };
		fog_r.particle_size = 8;

		Vec2D fl_min = { 0.0, -0.001 };
		Vec2D fl_max = { 0.01, -0.0005 };
		Particle_Emitter::Particle_Emitter fog_l;
		Particle_Emitter::init(&fog_l, 1500);
		fog_l.emitter_pos.x = 0;
		fog_l.emitter_pos.y = 512;
		fog_l.particle_color = { 167, 166, 157 };
		fog_l.particle_size = 8;

		Vec2D g_min = { 0.0, -0.001 };
		Vec2D g_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass;
		Particle_Emitter::init(&grass, 100);
		grass.emitter_pos.x = 25;
		grass.emitter_pos.y = 383;
		grass.particle_color = { 50, 205, 50 };
		grass.particle_size = 2;

		Vec2D g2_min = { 0.0, -0.001 };
		Vec2D g2_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass2;
		Particle_Emitter::init(&grass2, 100);
		grass2.emitter_pos.x = 50;
		grass2.emitter_pos.y = 383;
		grass2.particle_color = { 50, 205, 50 };
		grass2.particle_size = 4;

		Vec2D g3_min = { 0.0, -0.001 };
		Vec2D g3_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass3;
		Particle_Emitter::init(&grass3, 100);
		grass3.emitter_pos.x = 75;
		grass3.emitter_pos.y = 383;
		grass3.particle_color = { 50, 205, 50 };
		grass3.particle_size = 1;

		Vec2D g4_min = { 0.0, -0.001 };
		Vec2D g4_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass4;
		Particle_Emitter::init(&grass4, 100);
		grass4.emitter_pos.x = 100;
		grass4.emitter_pos.y = 383;
		grass4.particle_color = { 50, 205, 50 };
		grass4.particle_size = 5;

		Vec2D g5_min = { 0.0, -0.001 };
		Vec2D g5_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass5;
		Particle_Emitter::init(&grass5, 100);
		grass5.emitter_pos.x = 125;
		grass5.emitter_pos.y = 383;
		grass5.particle_color = { 50, 205, 50 };
		grass5.particle_size = 2;

		Vec2D g6_min = { 0.0, -0.001 };
		Vec2D g6_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass6;
		Particle_Emitter::init(&grass6, 100);
		grass6.emitter_pos.x = 150;
		grass6.emitter_pos.y = 383;
		grass6.particle_color = { 50, 205, 50 };
		grass6.particle_size = 1;

		Vec2D g7_min = { 0.0, -0.001 };
		Vec2D g7_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass7;
		Particle_Emitter::init(&grass7, 100);
		grass7.emitter_pos.x = 175;
		grass7.emitter_pos.y = 383;
		grass7.particle_color = { 50, 205, 50 };
		grass7.particle_size = 3;

		Vec2D g8_min = { 0.0, -0.001 };
		Vec2D g8_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass8;
		Particle_Emitter::init(&grass8, 100);
		grass8.emitter_pos.x = 200;
		grass8.emitter_pos.y = 383;
		grass8.particle_color = { 50, 205, 50 };
		grass8.particle_size = 4;

		Vec2D g9_min = { 0.0, -0.001 };
		Vec2D g9_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass9;
		Particle_Emitter::init(&grass9, 100);
		grass9.emitter_pos.x = 225;
		grass9.emitter_pos.y = 383;
		grass9.particle_color = { 50, 205, 50 };
		grass9.particle_size = 1;

		Vec2D g10_min = { 0.0, -0.001 };
		Vec2D g10_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass10;
		Particle_Emitter::init(&grass10, 100);
		grass10.emitter_pos.x = 250;
		grass10.emitter_pos.y = 383;
		grass10.particle_color = { 50, 205, 50 };
		grass10.particle_size = 3;

		Vec2D g11_min = { 0.0, -0.001 };
		Vec2D g11_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass11;
		Particle_Emitter::init(&grass11, 100);
		grass11.emitter_pos.x = 275;
		grass11.emitter_pos.y = 383;
		grass11.particle_color = { 50, 205, 50 };
		grass11.particle_size = 4;

		Vec2D g12_min = { 0.0, -0.001 };
		Vec2D g12_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass12;
		Particle_Emitter::init(&grass12, 100);
		grass12.emitter_pos.x = 300;
		grass12.emitter_pos.y = 383;
		grass12.particle_color = { 50, 205, 50 };
		grass12.particle_size = 2;

		Vec2D g13_min = { 0.0, -0.001 };
		Vec2D g13_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass13;
		Particle_Emitter::init(&grass13, 100);
		grass13.emitter_pos.x = 325;
		grass13.emitter_pos.y = 383;
		grass13.particle_color = { 50, 205, 50 };
		grass13.particle_size = 5;

		Vec2D g14_min = { 0.0, -0.001 };
		Vec2D g14_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass14;
		Particle_Emitter::init(&grass14, 100);
		grass14.emitter_pos.x = 350;
		grass14.emitter_pos.y = 383;
		grass14.particle_color = { 50, 205, 50 };
		grass14.particle_size = 1;

		Vec2D g15_min = { 0.0, -0.001 };
		Vec2D g15_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass15;
		Particle_Emitter::init(&grass15, 100);
		grass15.emitter_pos.x = 375;
		grass15.emitter_pos.y = 383;
		grass15.particle_color = { 50, 205, 50 };
		grass15.particle_size = 3;

		Vec2D g16_min = { 0.0, -0.001 };
		Vec2D g16_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass16;
		Particle_Emitter::init(&grass16, 100);
		grass16.emitter_pos.x = 400;
		grass16.emitter_pos.y = 383;
		grass16.particle_color = { 50, 205, 50 };
		grass16.particle_size = 2;

		Vec2D g17_min = { 0.0, -0.001 };
		Vec2D g17_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass17;
		Particle_Emitter::init(&grass17, 100);
		grass17.emitter_pos.x = 425;
		grass17.emitter_pos.y = 383;
		grass17.particle_color = { 50, 205, 50 };
		grass17.particle_size = 5;

		Vec2D g18_min = { 0.0, -0.001 };
		Vec2D g18_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass18;
		Particle_Emitter::init(&grass18, 100);
		grass18.emitter_pos.x = 450;
		grass18.emitter_pos.y = 383;
		grass18.particle_color = { 50, 205, 50 };
		grass18.particle_size = 4;

		Vec2D g19_min = { 0.0, -0.001 };
		Vec2D g19_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass19;
		Particle_Emitter::init(&grass19, 100);
		grass19.emitter_pos.x = 475;
		grass19.emitter_pos.y = 383;
		grass19.particle_color = { 50, 205, 50 };
		grass19.particle_size = 5;

		Vec2D g20_min = { 0.0, -0.001 };
		Vec2D g20_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass20;
		Particle_Emitter::init(&grass20, 100);
		grass20.emitter_pos.x = 500;
		grass20.emitter_pos.y = 383;
		grass20.particle_color = { 50, 205, 50 };
		grass20.particle_size = 3;

		Vec2D g21_min = { 0.0, -0.001 };
		Vec2D g21_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass21;
		Particle_Emitter::init(&grass21, 100);
		grass21.emitter_pos.x = 525;
		grass21.emitter_pos.y = 383;
		grass21.particle_color = { 50, 205, 50 };
		grass21.particle_size = 3;

		Vec2D g22_min = { 0.0, -0.001 };
		Vec2D g22_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass22;
		Particle_Emitter::init(&grass22, 100);
		grass22.emitter_pos.x = 550;
		grass22.emitter_pos.y = 383;
		grass22.particle_color = { 50, 205, 50 };
		grass22.particle_size = 1;

		Vec2D g23_min = { 0.0, -0.001 };
		Vec2D g23_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass23;
		Particle_Emitter::init(&grass23, 100);
		grass23.emitter_pos.x = 575;
		grass23.emitter_pos.y = 383;
		grass23.particle_color = { 50, 205, 50 };
		grass23.particle_size = 4;

		Vec2D g24_min = { 0.0, -0.001 };
		Vec2D g24_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass24;
		Particle_Emitter::init(&grass24, 100);
		grass24.emitter_pos.x = 600;
		grass24.emitter_pos.y = 383;
		grass24.particle_color = { 50, 205, 50 };
		grass24.particle_size = 2;

		Vec2D g25_min = { 0.0, -0.001 };
		Vec2D g25_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass25;
		Particle_Emitter::init(&grass25, 100);
		grass25.emitter_pos.x = 625;
		grass25.emitter_pos.y = 383;
		grass25.particle_color = { 50, 205, 50 };
		grass25.particle_size = 5;

		Vec2D g26_min = { 0.0, -0.001 };
		Vec2D g26_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass26;
		Particle_Emitter::init(&grass26, 100);
		grass26.emitter_pos.x = 650;
		grass26.emitter_pos.y = 383;
		grass26.particle_color = { 50, 205, 50 };
		grass26.particle_size = 1;

		Vec2D g27_min = { 0.0, -0.001 };
		Vec2D g27_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass27;
		Particle_Emitter::init(&grass27, 100);
		grass27.emitter_pos.x = 675;
		grass27.emitter_pos.y = 383;
		grass27.particle_color = { 50, 205, 50 };
		grass27.particle_size = 3;

		Vec2D g28_min = { 0.0, -0.001 };
		Vec2D g28_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass28;
		Particle_Emitter::init(&grass28, 100);
		grass28.emitter_pos.x = 700;
		grass28.emitter_pos.y = 383;
		grass28.particle_color = { 50, 205, 50 };
		grass28.particle_size = 2;

		Vec2D g29_min = { 0.0, -0.001 };
		Vec2D g29_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass29;
		Particle_Emitter::init(&grass29, 100);
		grass29.emitter_pos.x = 725;
		grass29.emitter_pos.y = 383;
		grass29.particle_color = { 50, 205, 50 };
		grass29.particle_size = 4;

		Vec2D g30_min = { 0.0, -0.001 };
		Vec2D g30_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass30;
		Particle_Emitter::init(&grass30, 100);
		grass30.emitter_pos.x = 750;
		grass30.emitter_pos.y = 383;
		grass30.particle_color = { 50, 205, 50 };
		grass30.particle_size = 5;

		Vec2D g31_min = { 0.0, -0.001 };
		Vec2D g31_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass31;
		Particle_Emitter::init(&grass31, 100);
		grass31.emitter_pos.x = 775;
		grass31.emitter_pos.y = 383;
		grass31.particle_color = { 50, 205, 50 };
		grass31.particle_size = 1;

		Vec2D g32_min = { 0.0, -0.001 };
		Vec2D g32_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass32;
		Particle_Emitter::init(&grass32, 100);
		grass32.emitter_pos.x = 800;
		grass32.emitter_pos.y = 383;
		grass32.particle_color = { 50, 205, 50 };
		grass32.particle_size = 4;

		Vec2D g33_min = { 0.0, -0.001 };
		Vec2D g33_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass33;
		Particle_Emitter::init(&grass33, 100);
		grass33.emitter_pos.x = 825;
		grass33.emitter_pos.y = 383;
		grass33.particle_color = { 50, 205, 50 };
		grass33.particle_size = 3;

		Vec2D g34_min = { 0.0, -0.001 };
		Vec2D g34_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass34;
		Particle_Emitter::init(&grass34, 100);
		grass34.emitter_pos.x = 850;
		grass34.emitter_pos.y = 383;
		grass34.particle_color = { 50, 205, 50 };
		grass34.particle_size = 1;

		Vec2D g35_min = { 0.0, -0.001 };
		Vec2D g35_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass35;
		Particle_Emitter::init(&grass35, 100);
		grass35.emitter_pos.x = 875;
		grass35.emitter_pos.y = 383;
		grass35.particle_color = { 50, 205, 50 };
		grass35.particle_size = 2;

		Vec2D g36_min = { 0.0, -0.001 };
		Vec2D g36_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass36;
		Particle_Emitter::init(&grass36, 100);
		grass36.emitter_pos.x = 900;
		grass36.emitter_pos.y = 383;
		grass36.particle_color = { 50, 205, 50 };
		grass36.particle_size = 5;

		Vec2D g37_min = { 0.0, -0.001 };
		Vec2D g37_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass37;
		Particle_Emitter::init(&grass37, 100);
		grass37.emitter_pos.x = 925;
		grass37.emitter_pos.y = 383;
		grass37.particle_color = { 50, 205, 50 };
		grass37.particle_size = 5;

		Vec2D g38_min = { 0.0, -0.001 };
		Vec2D g38_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass38;
		Particle_Emitter::init(&grass38, 100);
		grass38.emitter_pos.x = 950;
		grass38.emitter_pos.y = 383;
		grass38.particle_color = { 50, 205, 50 };
		grass38.particle_size = 4;

		Vec2D g39_min = { 0.0, -0.001 };
		Vec2D g39_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass39;
		Particle_Emitter::init(&grass39, 100);
		grass39.emitter_pos.x = 975;
		grass39.emitter_pos.y = 383;
		grass39.particle_color = { 50, 205, 50 };
		grass39.particle_size = 2;

		Vec2D g40_min = { 0.0, -0.001 };
		Vec2D g40_max = { 0.0001, -0.0005 };
		Particle_Emitter::Particle_Emitter grass40;
		Particle_Emitter::init(&grass40, 100);
		grass40.emitter_pos.x = 1000;
		grass40.emitter_pos.y = 383;
		grass40.particle_color = { 50, 205, 50 };
		grass40.particle_size = 3;

		Vec2D c_min = { 0.0, -0.001 };
		Vec2D c_max = { 0.01, -0.00 };
		Particle_Emitter::Particle_Emitter cloud;
		Particle_Emitter::init(&cloud, 400);
		cloud.emitter_pos.x = 0;
		cloud.emitter_pos.y = 75;
		cloud.particle_color = { 167, 166, 157 };
		cloud.particle_size = 8;

		Vec2D b_min = { 0.0004, 0.01 };
		Vec2D b_max = { -0.0004, 0.001 };
		Particle_Emitter::Particle_Emitter beam;
		Particle_Emitter::init(&beam, 4000);
		beam.emitter_pos.x = 500;
		beam.emitter_pos.y = -25;
		beam.particle_color = { 255, 255, 255 };
		beam.particle_size = 10;

		Vec2D st_min = { 0.0, -0.001 };
		Vec2D st_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star;
		Particle_Emitter::init(&star, 4000);
		star.emitter_pos.x = 25;
		star.emitter_pos.y = 75;
		star.particle_color = { 255, 215, 0 };
		star.particle_size = 4;

		Vec2D st2_min = { 0.0, -0.001 };
		Vec2D st2_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star2;
		Particle_Emitter::init(&star2, 4000);
		star2.emitter_pos.x = 50;
		star2.emitter_pos.y = 30;
		star2.particle_color = { 255, 215, 0 };
		star2.particle_size = 4;

		Vec2D st3_min = { 0.0, -0.001 };
		Vec2D st3_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star3;
		Particle_Emitter::init(&star3, 4000);
		star3.emitter_pos.x = 75;
		star3.emitter_pos.y = 75;
		star3.particle_color = { 255, 215, 0 };
		star3.particle_size = 4;

		Vec2D st4_min = { 0.0, -0.001 };
		Vec2D st4_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star4;
		Particle_Emitter::init(&star4, 4000);
		star4.emitter_pos.x = 100;
		star4.emitter_pos.y = 30;
		star4.particle_color = { 255, 215, 0 };
		star4.particle_size = 4;

		Vec2D st5_min = { 0.0, -0.001 };
		Vec2D st5_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star5;
		Particle_Emitter::init(&star5, 4000);
		star5.emitter_pos.x = 125;
		star5.emitter_pos.y = 75;
		star5.particle_color = { 255, 215, 0 };
		star5.particle_size = 4;

		Vec2D st6_min = { 0.0, -0.001 };
		Vec2D st6_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star6;
		Particle_Emitter::init(&star6, 4000);
		star6.emitter_pos.x = 150;
		star6.emitter_pos.y = 30;
		star6.particle_color = { 255, 215, 0 };
		star6.particle_size = 4;

		Vec2D st7_min = { 0.0, -0.001 };
		Vec2D st7_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star7;
		Particle_Emitter::init(&star7, 4000);
		star7.emitter_pos.x = 175;
		star7.emitter_pos.y = 75;
		star7.particle_color = { 255, 215, 0 };
		star7.particle_size = 4;

		Vec2D st8_min = { 0.0, -0.001 };
		Vec2D st8_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star8;
		Particle_Emitter::init(&star8, 4000);
		star8.emitter_pos.x = 200;
		star8.emitter_pos.y = 30;
		star8.particle_color = { 255, 215, 0 };
		star8.particle_size = 4;

		Vec2D st9_min = { 0.0, -0.001 };
		Vec2D st9_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star9;
		Particle_Emitter::init(&star9, 4000);
		star9.emitter_pos.x = 225;
		star9.emitter_pos.y = 75;
		star9.particle_color = { 255, 215, 0 };
		star9.particle_size = 4;

		Vec2D st10_min = { 0.0, -0.001 };
		Vec2D st10_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star10;
		Particle_Emitter::init(&star10, 4000);
		star10.emitter_pos.x = 250;
		star10.emitter_pos.y = 30;
		star10.particle_color = { 255, 215, 0 };

		Vec2D st11_min = { 0.0, -0.001 };
		Vec2D st11_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star11;
		Particle_Emitter::init(&star11, 4000);
		star11.emitter_pos.x = 275;
		star11.emitter_pos.y = 75;
		star11.particle_color = { 255, 215, 0 };
		star11.particle_size = 4;

		Vec2D st12_min = { 0.0, -0.001 };
		Vec2D st12_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star12;
		Particle_Emitter::init(&star12, 4000);
		star12.emitter_pos.x = 300;
		star12.emitter_pos.y = 30;
		star12.particle_color = { 255, 215, 0 };
		star12.particle_size = 4;

		Vec2D st13_min = { 0.0, -0.001 };
		Vec2D st13_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star13;
		Particle_Emitter::init(&star13, 4000);
		star13.emitter_pos.x = 325;
		star13.emitter_pos.y = 75;
		star13.particle_color = { 255, 215, 0 };
		star13.particle_size = 4;

		Vec2D st14_min = { 0.0, -0.001 };
		Vec2D st14_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star14;
		Particle_Emitter::init(&star14, 4000);
		star14.emitter_pos.x = 350;
		star14.emitter_pos.y = 30;
		star14.particle_color = { 255, 215, 0 };
		star14.particle_size = 4;

		Vec2D st15_min = { 0.0, -0.001 };
		Vec2D st15_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star15;
		Particle_Emitter::init(&star15, 4000);
		star15.emitter_pos.x = 375;
		star15.emitter_pos.y = 75;
		star15.particle_color = { 255, 215, 0 };
		star15.particle_size = 4;

		Vec2D st16_min = { 0.0, -0.001 };
		Vec2D st16_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star16;
		Particle_Emitter::init(&star16, 4000);
		star16.emitter_pos.x = 400;
		star16.emitter_pos.y = 30;
		star16.particle_color = { 255, 215, 0 };
		star16.particle_size = 4;

		Vec2D st17_min = { 0.0, -0.001 };
		Vec2D st17_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star17;
		Particle_Emitter::init(&star17, 4000);
		star17.emitter_pos.x = 425;
		star17.emitter_pos.y = 75;
		star17.particle_color = { 255, 215, 0 };
		star17.particle_size = 4;

		Vec2D st18_min = { 0.0, -0.001 };
		Vec2D st18_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star18;
		Particle_Emitter::init(&star18, 4000);
		star18.emitter_pos.x = 450;
		star18.emitter_pos.y = 30;
		star18.particle_color = { 255, 215, 0 };
		star18.particle_size = 4;

		Vec2D st19_min = { 0.0, -0.001 };
		Vec2D st19_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star19;
		Particle_Emitter::init(&star19, 4000);
		star19.emitter_pos.x = 475;
		star19.emitter_pos.y = 75;
		star19.particle_color = { 255, 215, 0 };
		star19.particle_size = 4;

		Vec2D st20_min = { 0.0, -0.001 };
		Vec2D st20_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star20;
		Particle_Emitter::init(&star20, 4000);
		star20.emitter_pos.x = 500;
		star20.emitter_pos.y = 30;
		star20.particle_color = { 255, 215, 0 };
		star20.particle_size = 4;

		Vec2D st21_min = { 0.0, -0.001 };
		Vec2D st21_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star21;
		Particle_Emitter::init(&star21, 4000);
		star21.emitter_pos.x = 525;
		star21.emitter_pos.y = 75;
		star21.particle_color = { 255, 215, 0 };
		star21.particle_size = 4;

		Vec2D st22_min = { 0.0, -0.001 };
		Vec2D st22_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star22;
		Particle_Emitter::init(&star22, 4000);
		star22.emitter_pos.x = 550;
		star22.emitter_pos.y = 30;
		star22.particle_color = { 255, 215, 0 };
		star22.particle_size = 4;

		Vec2D st23_min = { 0.0, -0.001 };
		Vec2D st23_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star23;
		Particle_Emitter::init(&star23, 4000);
		star23.emitter_pos.x = 575;
		star23.emitter_pos.y = 75;
		star23.particle_color = { 255, 215, 0 };
		star23.particle_size = 4;

		Vec2D st24_min = { 0.0, -0.001 };
		Vec2D st24_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star24;
		Particle_Emitter::init(&star24, 4000);
		star24.emitter_pos.x = 600;
		star24.emitter_pos.y = 30;
		star24.particle_color = { 255, 215, 0 };
		star24.particle_size = 4;

		Vec2D st25_min = { 0.0, -0.001 };
		Vec2D st25_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star25;
		Particle_Emitter::init(&star25, 4000);
		star25.emitter_pos.x = 625;
		star25.emitter_pos.y = 75;
		star25.particle_color = { 255, 215, 0 };
		star25.particle_size = 4;

		Vec2D st26_min = { 0.0, -0.001 };
		Vec2D st26_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star26;
		Particle_Emitter::init(&star26, 4000);
		star26.emitter_pos.x = 650;
		star26.emitter_pos.y = 30;
		star26.particle_color = { 255, 215, 0 };
		star26.particle_size = 4;

		Vec2D st27_min = { 0.0, -0.001 };
		Vec2D st27_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star27;
		Particle_Emitter::init(&star27, 4000);
		star27.emitter_pos.x = 675;
		star27.emitter_pos.y = 75;
		star27.particle_color = { 255, 215, 0 };
		star27.particle_size = 4;

		Vec2D st28_min = { 0.0, -0.001 };
		Vec2D st28_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star28;
		Particle_Emitter::init(&star28, 4000);
		star28.emitter_pos.x = 700;
		star28.emitter_pos.y = 30;
		star28.particle_color = { 255, 215, 0 };
		star28.particle_size = 4;

		Vec2D st29_min = { 0.0, -0.001 };
		Vec2D st29_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star29;
		Particle_Emitter::init(&star29, 4000);
		star29.emitter_pos.x = 725;
		star29.emitter_pos.y = 75;
		star29.particle_color = { 255, 215, 0 };
		star29.particle_size = 4;

		Vec2D st30_min = { 0.0, -0.001 };
		Vec2D st30_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star30;
		Particle_Emitter::init(&star30, 4000);
		star30.emitter_pos.x = 750;
		star30.emitter_pos.y = 30;
		star30.particle_color = { 255, 215, 0 };
		star30.particle_size = 4;
		star30.particle_size = 4;

		Vec2D st31_min = { 0.0, -0.001 };
		Vec2D st31_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star31;
		Particle_Emitter::init(&star31, 4000);
		star31.emitter_pos.x = 775;
		star31.emitter_pos.y = 75;
		star31.particle_color = { 255, 215, 0 };
		star31.particle_size = 4;
		star31.particle_size = 4;

		Vec2D st32_min = { 0.0, -0.001 };
		Vec2D st32_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star32;
		Particle_Emitter::init(&star32, 4000);
		star32.emitter_pos.x = 800;
		star32.emitter_pos.y = 30;
		star32.particle_color = { 255, 215, 0 };
		star32.particle_size = 4;
		star32.particle_size = 4;

		Vec2D st33_min = { 0.0, -0.001 };
		Vec2D st33_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star33;
		Particle_Emitter::init(&star33, 4000);
		star33.emitter_pos.x = 825;
		star33.emitter_pos.y = 75;
		star33.particle_color = { 255, 215, 0 };
		star33.particle_size = 4;
		star33.particle_size = 4;

		Vec2D st34_min = { 0.0, -0.001 };
		Vec2D st34_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star34;
		Particle_Emitter::init(&star34, 4000);
		star34.emitter_pos.x = 850;
		star34.emitter_pos.y = 30;
		star34.particle_color = { 255, 215, 0 };
		star34.particle_size = 4;
		star34.particle_size = 4;

		Vec2D st35_min = { 0.0, -0.001 };
		Vec2D st35_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star35;
		Particle_Emitter::init(&star35, 4000);
		star35.emitter_pos.x = 875;
		star35.emitter_pos.y = 75;
		star35.particle_color = { 255, 215, 0 };
		star35.particle_size = 4;
		star35.particle_size = 4;

		Vec2D st36_min = { 0.0, -0.001 };
		Vec2D st36_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star36;
		Particle_Emitter::init(&star36, 4000);
		star36.emitter_pos.x = 900;
		star36.emitter_pos.y = 30;
		star36.particle_color = { 255, 215, 0 };
		star36.particle_size = 4;
		star36.particle_size = 4;

		Vec2D st37_min = { 0.0, -0.001 };
		Vec2D st37_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star37;
		Particle_Emitter::init(&star37, 4000);
		star37.emitter_pos.x = 925;
		star37.emitter_pos.y = 75;
		star37.particle_color = { 255, 215, 0 };
		star37.particle_size = 4;
		star37.particle_size = 4;

		Vec2D st38_min = { 0.0, -0.001 };
		Vec2D st38_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star38;
		Particle_Emitter::init(&star38, 4000);
		star38.emitter_pos.x = 950;
		star38.emitter_pos.y = 30;
		star38.particle_color = { 255, 215, 0 };
		star38.particle_size = 4;
		star38.particle_size = 4;

		Vec2D st39_min = { 0.0, -0.001 };
		Vec2D st39_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star39;
		Particle_Emitter::init(&star39, 4000);
		star39.emitter_pos.x = 975;
		star39.emitter_pos.y = 75;
		star39.particle_color = { 255, 215, 0 };
		star39.particle_size = 4;
		star39.particle_size = 4;

		Vec2D st40_min = { 0.0, -0.001 };
		Vec2D st40_max = { 0.001, -0.0005 };
		Particle_Emitter::Particle_Emitter star40;
		Particle_Emitter::init(&star40, 4000);
		star40.emitter_pos.x = 1000;
		star40.emitter_pos.y = 30;
		star40.particle_color = { 255, 215, 0 };
		star40.particle_size = 4;
		star40.particle_size = 4;

		//screen setup
		SDL_Init(SDL_INIT_VIDEO);

		SDL_ShowCursor(SDL_DISABLE);

		int screen_width = 32 * 32;
		int screen_height = 32 * 16;

		SDL_Window *window = SDL_CreateWindow(
			"Thriller Night",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			screen_width, screen_height, SDL_WINDOW_SHOWN);

		SDL_Renderer *renderer = SDL_CreateRenderer(window,
			-1, SDL_RENDERER_ACCELERATED);

		//sound

		int soundTime = 0;
		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

		Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 2048);

		Mix_Music *music = Mix_LoadMUS("thriller.mp3");

		Mix_Chunk *fx = Mix_LoadWAV("price.wav");

		Mix_PlayChannel(-1, fx, 0);

		Mix_PlayMusic(music, -1);

		//backdrop
		SDL_Surface *backround = IMG_Load("backround.png");
		SDL_Texture *texture3 = SDL_CreateTextureFromSurface(renderer, backround);
		SDL_Rect scroll;
		scroll.x = 0;
		scroll.y = 0;
		scroll.w = 2048;
		scroll.h = 512;

		//falling stars
		Pos fStar_pos;
		Speed fStar_speed;
		Size fStar_size;
		RGB fStar_color;

		fStar_pos.x = 0;
		fStar_pos.y = 0;
		fStar_size.w = 8;
		fStar_size.h = 8;
		fStar_speed.x = 0.4;
		fStar_speed.y = 0.5;
		fStar_color.r = 255;
		fStar_color.g = 215;
		fStar_color.b = 0;

		Pos fStar2_pos;
		Speed fStar2_speed;
		Size fStar2_size;
		RGB fStar2_color;

		fStar2_pos.x = 350;
		fStar2_pos.y = 0;
		fStar2_size.w = 8;
		fStar2_size.h = 8;
		fStar2_speed.x = 0.6;
		fStar2_speed.y = 0.3;
		fStar2_color.r = 255;
		fStar2_color.g = 215;
		fStar2_color.b = 0;

		Pos fStar3_pos;
		Speed fStar3_speed;
		Size fStar3_size;
		RGB fStar3_color;

		fStar3_pos.x = 0;
		fStar3_pos.y = 0;
		fStar3_size.w = 8;
		fStar3_size.h = 8;
		fStar3_speed.x = 0.2;
		fStar3_speed.y = 0.6;
		fStar3_color.r = 255;
		fStar3_color.g = 215;
		fStar3_color.b = 0;

		//Sprites
		SDL_Surface *surface2 = IMG_Load("jack.png");
		SDL_Texture *texture2 = SDL_CreateTextureFromSurface(renderer, surface2);
		SDL_FreeSurface(surface2);

		int first_frame_pos_x = 0;
		int first_frame_pos_y = 1475;
		int frame_w = 45;
		int frame_h = 64;
		int n_frames = 20;
		int frame_duration = 100;
		unsigned int last_frame_change_time = 1;
		int current_frame = 0;

		//tilemap
		SDL_Surface *surface = IMG_Load("tileset.png");
		SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);

		Tileset::Tileset tileset;
		tileset.tile_w = 32;
		tileset.tile_h = 32;
		tileset.n_cols = 23;

		Table_File::Table_File map_file;
		Table_File::read("party_map.csv", &map_file);

		int **map = new int*[map_file.nrows];
		for (int i = 0; i < map_file.nrows; i++)
		{
			map[i] = new int[map_file.ncols[i]];

			for (int j = 0; j < map_file.ncols[i]; j++)
			{
				map[i][j] = atoi(map_file.table[i][j]);
			}
		}
		int map_n_cols = map_file.ncols[0];
		int map_n_rows = map_file.nrows;

		//key press
		unsigned char prev_key_state[256];
		unsigned char *keys = (unsigned char*)SDL_GetKeyboardState(NULL);

		//init
		bool done = false;
		while (!done)
		{
			memcpy(prev_key_state, keys, 256);
			unsigned int current_time = SDL_GetTicks();

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT)
				{
					done = true;
				}
			}

			if (current_time - last_frame_change_time >= frame_duration)
			{
				current_frame = (current_frame + 1) % n_frames;
				last_frame_change_time = current_time;
				scroll.x -= 1;
				if (scroll.x <= -1024)
				{
					scroll.x = 0;
				}
				
			}
			//key presses
			int cmd_end = 0;

			//game update
			if (keys[SDL_SCANCODE_E]) cmd_end = 1;
			
			fStar_pos.x += fStar_speed.x;
			fStar_pos.y += fStar_speed.y;

			if (fStar_pos.x < 0 || fStar_pos.x + fStar_size.w > screen_width)
			{
				fStar_speed.x *= -1;
			}
			if (fStar_pos.y < 0 || fStar_pos.y + fStar_size.h > screen_height)
			{
				fStar_speed.y *= -1;

			}

			if (keys[SDL_SCANCODE_E]) cmd_end = 1;

			fStar2_pos.x += fStar2_speed.x;
			fStar2_pos.y += fStar2_speed.y;

			if (fStar2_pos.x < 0 || fStar2_pos.x + fStar2_size.w > screen_width)
			{
				fStar2_speed.x *= -1;
			}
			if (fStar2_pos.y < 0 || fStar2_pos.y + fStar2_size.h > screen_height)
			{
				fStar2_speed.y *= -1;

			}

			if (keys[SDL_SCANCODE_E]) cmd_end = 1;

			fStar3_pos.x += fStar3_speed.x;
			fStar3_pos.y += fStar3_speed.y;

			if (fStar3_pos.x < 0 || fStar3_pos.x + fStar3_size.w > screen_width)
			{
				fStar3_speed.x *= -1;
			}
			if (fStar3_pos.y < 0 || fStar3_pos.y + fStar3_size.h > screen_height)
			{
				fStar3_speed.y *= -1;

			}
			
				//Emitters update
			Particle_Emitter::clear_Forces_from_Particles(&torch);
			Particle_Emitter::spawn_Many(&torch, t_min, t_max, 1, 600, 1000);
			Particle_Emitter::update(&torch, 1.0);

			Particle_Emitter::clear_Forces_from_Particles(&torch2);
			Particle_Emitter::spawn_Many(&torch2, t2_min, t2_max, 1, 600, 1000);
			Particle_Emitter::update(&torch2, 1.0);

			Particle_Emitter::clear_Forces_from_Particles(&sparkler);
			Particle_Emitter::spawn_Many(&sparkler, s_min, s_max, 1, 300, 10000);
			Particle_Emitter::update(&sparkler, 10.0);

			Particle_Emitter::clear_Forces_from_Particles(&sparkler2);
			Particle_Emitter::spawn_Many(&sparkler2, s2_min, s2_max, 1, 300, 10000);
			Particle_Emitter::update(&sparkler2, 10.0);

			Particle_Emitter::clear_Forces_from_Particles(&fog_r);
			Particle_Emitter::spawn_Many(&fog_r, fr_min, fr_max, 1, 500, 15000);
			Particle_Emitter::update(&fog_r, 1.0);

			Particle_Emitter::clear_Forces_from_Particles(&fog_l);
			Particle_Emitter::spawn_Many(&fog_l, fl_min, fl_max, 1, 500, 15000);
			Particle_Emitter::update(&fog_l, 1.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass);
			Particle_Emitter::spawn_Many(&grass, g_min, g_max, 1, 500, 700);
			Particle_Emitter::update(&grass, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass2);
			Particle_Emitter::spawn_Many(&grass2, g2_min, g2_max, 1, 500, 700);
			Particle_Emitter::update(&grass2, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass3);
			Particle_Emitter::spawn_Many(&grass3, g3_min, g3_max, 1, 500, 700);
			Particle_Emitter::update(&grass3, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass4);
			Particle_Emitter::spawn_Many(&grass4, g4_min, g4_max, 1, 500, 700);
			Particle_Emitter::update(&grass4, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass5);
			Particle_Emitter::spawn_Many(&grass5, g5_min, g5_max, 1, 500, 700);
			Particle_Emitter::update(&grass5, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass6);
			Particle_Emitter::spawn_Many(&grass6, g6_min, g6_max, 1, 500, 700);
			Particle_Emitter::update(&grass6, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass7);
			Particle_Emitter::spawn_Many(&grass7, g7_min, g7_max, 1, 500, 700);
			Particle_Emitter::update(&grass7, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass8);
			Particle_Emitter::spawn_Many(&grass8, g8_min, g8_max, 1, 500, 700);
			Particle_Emitter::update(&grass8, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass9);
			Particle_Emitter::spawn_Many(&grass9, g9_min, g9_max, 1, 500, 700);
			Particle_Emitter::update(&grass9, 2.0);
			
			Particle_Emitter::clear_Forces_from_Particles(&grass10);
			Particle_Emitter::spawn_Many(&grass10, g10_min, g10_max, 1, 500, 700);
			Particle_Emitter::update(&grass10, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass11);
			Particle_Emitter::spawn_Many(&grass11, g11_min, g11_max, 1, 500, 700);
			Particle_Emitter::update(&grass11, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass12);
			Particle_Emitter::spawn_Many(&grass12, g12_min, g12_max, 1, 500, 700);
			Particle_Emitter::update(&grass12, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass13);
			Particle_Emitter::spawn_Many(&grass13, g13_min, g13_max, 1, 500, 700);
			Particle_Emitter::update(&grass13, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass14);
			Particle_Emitter::spawn_Many(&grass14, g14_min, g14_max, 1, 500, 700);
			Particle_Emitter::update(&grass14, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass15);
			Particle_Emitter::spawn_Many(&grass15, g15_min, g15_max, 1, 500, 700);
			Particle_Emitter::update(&grass15, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass16);
			Particle_Emitter::spawn_Many(&grass16, g16_min, g16_max, 1, 500, 700);
			Particle_Emitter::update(&grass16, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass17);
			Particle_Emitter::spawn_Many(&grass17, g17_min, g17_max, 1, 500, 700);
			Particle_Emitter::update(&grass17, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass18);
			Particle_Emitter::spawn_Many(&grass18, g18_min, g18_max, 1, 500, 700);
			Particle_Emitter::update(&grass18, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass19);
			Particle_Emitter::spawn_Many(&grass19, g19_min, g19_max, 1, 500, 700);
			Particle_Emitter::update(&grass19, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass20);
			Particle_Emitter::spawn_Many(&grass20, g20_min, g20_max, 1, 500, 700);
			Particle_Emitter::update(&grass20, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass21);
			Particle_Emitter::spawn_Many(&grass21, g21_min, g21_max, 1, 500, 700);
			Particle_Emitter::update(&grass21, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass22);
			Particle_Emitter::spawn_Many(&grass22, g22_min, g22_max, 1, 500, 700);
			Particle_Emitter::update(&grass22, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass23);
			Particle_Emitter::spawn_Many(&grass23, g23_min, g23_max, 1, 500, 700);
			Particle_Emitter::update(&grass23, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass24);
			Particle_Emitter::spawn_Many(&grass24, g24_min, g24_max, 1, 500, 700);
			Particle_Emitter::update(&grass24, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass25);
			Particle_Emitter::spawn_Many(&grass25, g25_min, g25_max, 1, 500, 700);
			Particle_Emitter::update(&grass25, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass26);
			Particle_Emitter::spawn_Many(&grass26, g26_min, g26_max, 1, 500, 700);
			Particle_Emitter::update(&grass26, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass27);
			Particle_Emitter::spawn_Many(&grass27, g27_min, g27_max, 1, 500, 700);
			Particle_Emitter::update(&grass27, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass28);
			Particle_Emitter::spawn_Many(&grass28, g28_min, g28_max, 1, 500, 700);
			Particle_Emitter::update(&grass28, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass29);
			Particle_Emitter::spawn_Many(&grass29, g29_min, g29_max, 1, 500, 700);
			Particle_Emitter::update(&grass29, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass30);
			Particle_Emitter::spawn_Many(&grass30, g30_min, g30_max, 1, 500, 700);
			Particle_Emitter::update(&grass30, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass31);
			Particle_Emitter::spawn_Many(&grass31, g31_min, g31_max, 1, 500, 700);
			Particle_Emitter::update(&grass31, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass32);
			Particle_Emitter::spawn_Many(&grass32, g32_min, g32_max, 1, 500, 700);
			Particle_Emitter::update(&grass32, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass33);
			Particle_Emitter::spawn_Many(&grass33, g33_min, g33_max, 1, 500, 700);
			Particle_Emitter::update(&grass33, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass34);
			Particle_Emitter::spawn_Many(&grass34, g34_min, g34_max, 1, 500, 700);
			Particle_Emitter::update(&grass34, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass35);
			Particle_Emitter::spawn_Many(&grass35, g35_min, g35_max, 1, 500, 700);
			Particle_Emitter::update(&grass35, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass36);
			Particle_Emitter::spawn_Many(&grass36, g36_min, g36_max, 1, 500, 700);
			Particle_Emitter::update(&grass36, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass37);
			Particle_Emitter::spawn_Many(&grass37, g37_min, g37_max, 1, 500, 700);
			Particle_Emitter::update(&grass37, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass38);
			Particle_Emitter::spawn_Many(&grass38, g38_min, g38_max, 1, 500, 700);
			Particle_Emitter::update(&grass38, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&grass39);
			Particle_Emitter::spawn_Many(&grass39, g39_min, g39_max, 1, 500, 700);
			Particle_Emitter::update(&grass39, 2.0);


			Particle_Emitter::clear_Forces_from_Particles(&grass40);
			Particle_Emitter::spawn_Many(&grass40, g40_min, g40_max, 1, 500, 700);
			Particle_Emitter::update(&grass40, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&cloud);
			Particle_Emitter::spawn_Many(&cloud, c_min, c_max, 1, 1000, 15000);
			Particle_Emitter::update(&cloud, 2.0);

			Particle_Emitter::clear_Forces_from_Particles(&beam);
			Particle_Emitter::spawn_Many(&beam, b_min, b_max, 1, 100, 15000);
			Particle_Emitter::update(&beam, 6.0);

			Particle_Emitter::clear_Forces_from_Particles(&star);
			Particle_Emitter::spawn_Many(&star, st_min, st_max, 1, 10, 1000);
			Particle_Emitter::update(&star, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star2);
			Particle_Emitter::spawn_Many(&star2, st2_min, st2_max, 1, 10, 1000);
			Particle_Emitter::update(&star2, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star3);
			Particle_Emitter::spawn_Many(&star3, st3_min, st3_max, 1, 10, 1000);
			Particle_Emitter::update(&star3, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star4);
			Particle_Emitter::spawn_Many(&star4, st4_min, st4_max, 1, 10, 1000);
			Particle_Emitter::update(&star4, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star5);
			Particle_Emitter::spawn_Many(&star5, st5_min, st5_max, 1, 10, 1000);
			Particle_Emitter::update(&star5, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star6);
			Particle_Emitter::spawn_Many(&star6, st6_min, st6_max, 1, 10, 1000);
			Particle_Emitter::update(&star6, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star7);
			Particle_Emitter::spawn_Many(&star7, st7_min, st7_max, 1, 10, 1000);
			Particle_Emitter::update(&star7, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star8);
			Particle_Emitter::spawn_Many(&star8, st8_min, st8_max, 1, 10, 1000);
			Particle_Emitter::update(&star8, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star9);
			Particle_Emitter::spawn_Many(&star9, st9_min, st9_max, 1, 10, 1000);
			Particle_Emitter::update(&star9, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star10);
			Particle_Emitter::spawn_Many(&star10, st10_min, st10_max, 1, 10, 1000);
			Particle_Emitter::update(&star10, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star11);
			Particle_Emitter::spawn_Many(&star11, st11_min, st11_max, 1, 10, 1000);
			Particle_Emitter::update(&star11, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star12);
			Particle_Emitter::spawn_Many(&star12, st12_min, st12_max, 1, 10, 1000);
			Particle_Emitter::update(&star12, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star13);
			Particle_Emitter::spawn_Many(&star13, st13_min, st13_max, 1, 10, 1000);
			Particle_Emitter::update(&star13, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star14);
			Particle_Emitter::spawn_Many(&star14, st14_min, st14_max, 1, 10, 1000);
			Particle_Emitter::update(&star14, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star15);
			Particle_Emitter::spawn_Many(&star15, st15_min, st15_max, 1, 10, 1000);
			Particle_Emitter::update(&star15, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star16);
			Particle_Emitter::spawn_Many(&star16, st16_min, st16_max, 1, 10, 1000);
			Particle_Emitter::update(&star16, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star17);
			Particle_Emitter::spawn_Many(&star17, st17_min, st17_max, 1, 10, 1000);
			Particle_Emitter::update(&star17, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star18);
			Particle_Emitter::spawn_Many(&star18, st18_min, st18_max, 1, 10, 1000);
			Particle_Emitter::update(&star18, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star19);
			Particle_Emitter::spawn_Many(&star19, st19_min, st19_max, 1, 10, 1000);
			Particle_Emitter::update(&star19, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star20);
			Particle_Emitter::spawn_Many(&star20, st20_min, st20_max, 1, 10, 1000);
			Particle_Emitter::update(&star20, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star21);
			Particle_Emitter::spawn_Many(&star21, st21_min, st21_max, 1, 10, 1000);
			Particle_Emitter::update(&star21, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star22);
			Particle_Emitter::spawn_Many(&star22, st22_min, st22_max, 1, 10, 1000);
			Particle_Emitter::update(&star22, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star23);
			Particle_Emitter::spawn_Many(&star23, st23_min, st23_max, 1, 10, 1000);
			Particle_Emitter::update(&star23, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star24);
			Particle_Emitter::spawn_Many(&star24, st24_min, st24_max, 1, 10, 1000);
			Particle_Emitter::update(&star24, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star25);
			Particle_Emitter::spawn_Many(&star25, st25_min, st25_max, 1, 10, 1000);
			Particle_Emitter::update(&star25, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star26);
			Particle_Emitter::spawn_Many(&star26, st26_min, st26_max, 1, 10, 1000);
			Particle_Emitter::update(&star26, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star27);
			Particle_Emitter::spawn_Many(&star27, st27_min, st27_max, 1, 10, 1000);
			Particle_Emitter::update(&star27, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star28);
			Particle_Emitter::spawn_Many(&star28, st28_min, st28_max, 1, 10, 1000);
			Particle_Emitter::update(&star28, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star29);
			Particle_Emitter::spawn_Many(&star29, st29_min, st29_max, 1, 10, 1000);
			Particle_Emitter::update(&star29, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star30);
			Particle_Emitter::spawn_Many(&star30, st30_min, st30_max, 1, 10, 1000);
			Particle_Emitter::update(&star30, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star31);
			Particle_Emitter::spawn_Many(&star31, st31_min, st31_max, 1, 10, 1000);
			Particle_Emitter::update(&star31, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star32);
			Particle_Emitter::spawn_Many(&star32, st32_min, st32_max, 1, 10, 1000);
			Particle_Emitter::update(&star32, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star33);
			Particle_Emitter::spawn_Many(&star33, st33_min, st33_max, 1, 10, 1000);
			Particle_Emitter::update(&star33, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star34);
			Particle_Emitter::spawn_Many(&star34, st34_min, st34_max, 1, 10, 1000);
			Particle_Emitter::update(&star34, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star35);
			Particle_Emitter::spawn_Many(&star35, st35_min, st35_max, 1, 10, 1000);
			Particle_Emitter::update(&star35, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star36);
			Particle_Emitter::spawn_Many(&star36, st36_min, st36_max, 1, 10, 1000);
			Particle_Emitter::update(&star36, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star37);
			Particle_Emitter::spawn_Many(&star37, st37_min, st37_max, 1, 10, 1000);
			Particle_Emitter::update(&star37, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star38);
			Particle_Emitter::spawn_Many(&star38, st38_min, st38_max, 1, 10, 1000);
			Particle_Emitter::update(&star38, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star39);
			Particle_Emitter::spawn_Many(&star39, st39_min, st39_max, 1, 10, 1000);
			Particle_Emitter::update(&star39, 0.8);

			Particle_Emitter::clear_Forces_from_Particles(&star40);
			Particle_Emitter::spawn_Many(&star40, st40_min, st40_max, 1, 10, 1000);
			Particle_Emitter::update(&star40, 0.8);

				//DRAW
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);

				SDL_RenderCopyEx(renderer, texture3, &scroll, &scroll, 0, NULL, SDL_FLIP_NONE);
				
					//sprite block
					SDL_Rect src2;
					src2.x = first_frame_pos_x + current_frame * frame_w;
					src2.y = first_frame_pos_y;
					src2.w = frame_w;
					src2.h = frame_h;

					SDL_Rect dest2;
					dest2.x = 490;
					dest2.y = 256;
					dest2.w = frame_w;
					dest2.h = frame_h;

					//collison block
					SDL_Rect floor;
					floor.x = 0;
					floor.y = 385;
					floor.w = 1024;
					floor.h = 5;

					{
						int result = Collision::minkowski(
							fStar_pos.x, fStar_pos.y, fStar_size.w, fStar_size.h,
							floor.x, floor.y, floor.w, floor.h);

						if (result == Collision::LEFT_OF_1 || result == Collision::RIGHT_OF_1)
						{
							fStar_speed.x *= -1;
						}
						else if (result == Collision::BOTTOM_OF_1 || result == Collision::TOP_OF_1)
						{
							fStar_speed.y *= -1;
						}
					}
					{
						int result = Collision::minkowski(
							fStar2_pos.x, fStar2_pos.y, fStar2_size.w, fStar2_size.h,
							floor.x, floor.y, floor.w, floor.h);

						if (result == Collision::LEFT_OF_1 || result == Collision::RIGHT_OF_1)
						{
							fStar2_speed.x *= -1;
						}
						else if (result == Collision::BOTTOM_OF_1 || result == Collision::TOP_OF_1)
						{
							fStar2_speed.y *= -1;
						}
					}
					{
						int result = Collision::minkowski(
							fStar3_pos.x, fStar3_pos.y, fStar3_size.w, fStar3_size.h,
							floor.x, floor.y, floor.w, floor.h);

						if (result == Collision::LEFT_OF_1 || result == Collision::RIGHT_OF_1)
						{
							fStar3_speed.x *= -1;
						}
						else if (result == Collision::BOTTOM_OF_1 || result == Collision::TOP_OF_1)
						{
							fStar3_speed.y *= -1;
						}
					}

					//end game trigger
					int tempTrigger = 0;
					static int trigger = 0;
					if (cmd_end == 1)
					{
						tempTrigger = 1;
						trigger = 1;
					}
					//end game
					if (tempTrigger == 1)
					{
						_sleep(10);
						Mix_PlayChannel(-1, fx, 0);
					}
					if (trigger == 1)
					{
						dest2.y -= 1000;
						Particle_Emitter::clear_Forces_from_Particles(&beam);
						Particle_Emitter::draw(&beam, renderer);

						Particle_Emitter::spawn_Many(&sparkler, s_min, s_max, 10, 300, 10000);
						Particle_Emitter::update(&sparkler, 0.0004);

						Particle_Emitter::spawn_Many(&sparkler2, s2_min, s2_max, 10, 300, 10000);
						Particle_Emitter::update(&sparkler2, 0.0004);

						Particle_Emitter::spawn_Many(&torch, t_min, t_max, 10, 300, 10000);
						Particle_Emitter::update(&torch, 0.0004);

						Particle_Emitter::spawn_Many(&torch2, t2_min, t2_max, 10, 300, 10000);
						Particle_Emitter::update(&torch2, 0.0004);

						Particle_Emitter::spawn_Many(&beam, b_min, b_max, 20, 100, 15000);
						Particle_Emitter::update(&beam, 0.00004);
						Mix_HaltMusic();

					}

					SDL_RenderCopyEx(renderer, texture2, &src2, &dest2, 0, NULL, SDL_FLIP_NONE);
					
					SDL_RenderFillRect(renderer, &floor);

					for (int i = 0; i < map_n_rows; i++)
					{
						for (int j = 0; j < map_n_cols; j++)
						{
							int tile_id = map[i][j];

							SDL_Rect src;
							src.x = Tileset::get_Col(tile_id, &tileset);
							src.y = Tileset::get_Row(tile_id, &tileset);
							src.w = 32;
							src.h = 32;

							SDL_Rect dest;
							dest.x = j * tileset.tile_w;
							dest.y = i * tileset.tile_h;
							dest.w = 32;
							dest.h = 32;

							SDL_RenderCopyEx(renderer, texture, &src, &dest, 0, NULL, SDL_FLIP_NONE);
						}
				}

					SDL_SetRenderDrawColor(renderer, fStar_color.r, fStar_color.g, fStar_color.b, 255);
					SDL_Rect rect = { fStar_pos.x,fStar_pos.y,fStar_size.w,fStar_size.h };
					SDL_RenderFillRect(renderer, &rect);

					SDL_SetRenderDrawColor(renderer, fStar2_color.r, fStar2_color.g, fStar2_color.b, 255);
					SDL_Rect rect2 = { fStar2_pos.x,fStar2_pos.y,fStar2_size.w,fStar2_size.h };
					SDL_RenderFillRect(renderer, &rect2);

					SDL_SetRenderDrawColor(renderer, fStar3_color.r, fStar3_color.g, fStar3_color.b, 255);
					SDL_Rect rect3 = { fStar3_pos.x,fStar3_pos.y,fStar3_size.w,fStar3_size.h };
					SDL_RenderFillRect(renderer, &rect3);

					Particle_Emitter::draw(&star, renderer);
					Particle_Emitter::draw(&star2, renderer);
					Particle_Emitter::draw(&star3, renderer);
					Particle_Emitter::draw(&star4, renderer);
					Particle_Emitter::draw(&star5, renderer);
					Particle_Emitter::draw(&star6, renderer);
					Particle_Emitter::draw(&star7, renderer);
					Particle_Emitter::draw(&star8, renderer);
					Particle_Emitter::draw(&star9, renderer);
					Particle_Emitter::draw(&star10, renderer);
					Particle_Emitter::draw(&star11, renderer);
					Particle_Emitter::draw(&star12, renderer);
					Particle_Emitter::draw(&star13, renderer);
					Particle_Emitter::draw(&star14, renderer);
					Particle_Emitter::draw(&star15, renderer);
					Particle_Emitter::draw(&star16, renderer);
					Particle_Emitter::draw(&star17, renderer);
					Particle_Emitter::draw(&star18, renderer);
					Particle_Emitter::draw(&star19, renderer);
					Particle_Emitter::draw(&star20, renderer);
					Particle_Emitter::draw(&star21, renderer);
					Particle_Emitter::draw(&star22, renderer);
					Particle_Emitter::draw(&star23, renderer);
					Particle_Emitter::draw(&star24, renderer);
					Particle_Emitter::draw(&star25, renderer);
					Particle_Emitter::draw(&star26, renderer);
					Particle_Emitter::draw(&star27, renderer);
					Particle_Emitter::draw(&star28, renderer);
					Particle_Emitter::draw(&star29, renderer);
					Particle_Emitter::draw(&star30, renderer);
					Particle_Emitter::draw(&star31, renderer);
					Particle_Emitter::draw(&star32, renderer);
					Particle_Emitter::draw(&star33, renderer);
					Particle_Emitter::draw(&star34, renderer);
					Particle_Emitter::draw(&star35, renderer);
					Particle_Emitter::draw(&star36, renderer);
					Particle_Emitter::draw(&star37, renderer);
					Particle_Emitter::draw(&star38, renderer);
					Particle_Emitter::draw(&star39, renderer);
					Particle_Emitter::draw(&star40, renderer);
					Particle_Emitter::draw(&torch, renderer);
					Particle_Emitter::draw(&torch2, renderer);
					Particle_Emitter::draw(&sparkler, renderer);
					Particle_Emitter::draw(&sparkler2, renderer);
					Particle_Emitter::draw(&fog_r, renderer);
					Particle_Emitter::draw(&fog_l, renderer);
					Particle_Emitter::draw(&grass, renderer);
					Particle_Emitter::draw(&grass2, renderer);
					Particle_Emitter::draw(&grass3, renderer);
					Particle_Emitter::draw(&grass4, renderer);
					Particle_Emitter::draw(&grass5, renderer);
					Particle_Emitter::draw(&grass6, renderer);
					Particle_Emitter::draw(&grass7, renderer);
					Particle_Emitter::draw(&grass8, renderer);
					Particle_Emitter::draw(&grass9, renderer);
					Particle_Emitter::draw(&grass10, renderer);
					Particle_Emitter::draw(&grass11, renderer);
					Particle_Emitter::draw(&grass12, renderer);
					Particle_Emitter::draw(&grass13, renderer);
					Particle_Emitter::draw(&grass14, renderer);
					Particle_Emitter::draw(&grass15, renderer);
					Particle_Emitter::draw(&grass16, renderer);
					Particle_Emitter::draw(&grass17, renderer);
					Particle_Emitter::draw(&grass18, renderer);
					Particle_Emitter::draw(&grass19, renderer);
					Particle_Emitter::draw(&grass20, renderer);
					Particle_Emitter::draw(&grass21, renderer);
					Particle_Emitter::draw(&grass22, renderer);
					Particle_Emitter::draw(&grass23, renderer);
					Particle_Emitter::draw(&grass24, renderer);
					Particle_Emitter::draw(&grass25, renderer);
					Particle_Emitter::draw(&grass26, renderer);
					Particle_Emitter::draw(&grass27, renderer);
					Particle_Emitter::draw(&grass28, renderer);
					Particle_Emitter::draw(&grass29, renderer);
					Particle_Emitter::draw(&grass30, renderer);
					Particle_Emitter::draw(&grass31, renderer);
					Particle_Emitter::draw(&grass32, renderer);
					Particle_Emitter::draw(&grass33, renderer);
					Particle_Emitter::draw(&grass34, renderer);
					Particle_Emitter::draw(&grass35, renderer);
					Particle_Emitter::draw(&grass36, renderer);
					Particle_Emitter::draw(&grass37, renderer);
					Particle_Emitter::draw(&grass38, renderer);
					Particle_Emitter::draw(&grass39, renderer);
					Particle_Emitter::draw(&grass40, renderer);
					Particle_Emitter::draw(&cloud, renderer);

				SDL_RenderPresent(renderer);
			}

			return 0;
		}