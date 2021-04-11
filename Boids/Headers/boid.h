#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <vector>

const int PERCEPTION_RADIUS_COHESION = 20;
const int PERCEPTION_RADIUS_ALIGNMENT = 20;
const int PERCEPTION_RADIUS_SEPARATION = 10;

const float WEIGHT_FACTOR_RADIUS_TOCENTER_FORCE = 25.0f;
const float MAX_FORCE_MAGNITUDE = 1.0f;
const float MAX_SPEED = 10.0f;

class Boid
{
public:
	Boid(glm::vec3 position = glm::vec3(0.0f), glm::vec3 velocity = glm::vec3(1.0f)) {
		this->Position = position;
		this->Velocity = velocity;
		this->Acceleration = glm::vec3(0.0f);
	}

	void ApplyForce(glm::vec3 force) {
		this->Acceleration += force;
	}

	void ResetForce() {
		this->Acceleration *= 0.0f;
	}

	void Update(float deltaTime) {
		this->Position = this->Position + this->Velocity * deltaTime;
		this->Velocity = this->Velocity + this->Acceleration * deltaTime;
		// this->Velocity = this->limit(this->Velocity, this->MaxSpeed);
		// this->Acceleration *= 0;

		this->Model = glm::inverse(glm::lookAt(this->Position, this->Position + this->Velocity, glm::vec3(0.0f, 1.0f, 0.0f)));
		// this->Model = glm::rotate(this->Model, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));

		/*
		float norma = sqrt(this->Velocity.x * this->Velocity.x + this->Velocity.y * this->Velocity.y);
		float turn1 = acos(this->Velocity.x / norma) * 180 / M_PI;
		float turn2 = asin(this->Velocity.y / norma) * 180 / M_PI;
		this->Turn = turn1;
		if (turn2 < 0) this->Turn = -turn1;
		this->Turn = this->Turn - 90;
		*/
	}

	glm::vec3 Cohesion(std::vector<Boid> boids, float atten) {
		// Cohesion
		unsigned int neighbors = 0;
		glm::vec3 sum_position = glm::vec3(0.0f);

		for (int i = 0; i < boids.size(); i++) {
			float distance = glm::distance(this->Position, boids[i].Position);
			if ((distance > 0) && (distance < PERCEPTION_RADIUS_COHESION)) {
				sum_position += boids[i].Position;
				neighbors++;
			}
		}

		glm::vec3 force = glm::vec3(0.0f, 0.0f, 0.0f);
		if (neighbors > 0) {
			force = sum_position / static_cast<float>(neighbors);
		}

		// Limit Force
		this->LimitForce(force, MAX_FORCE_MAGNITUDE);

		return force;
	}

	glm::vec3 Alignment(std::vector<Boid> boids, float atten) {
		// Alignment
		unsigned int neighbors = 0;
		glm::vec3 sum_velocity = glm::vec3(0.0f);

		for (int i = 0; i < boids.size(); i++) {
			float distance = glm::distance(this->Position, boids[i].Position);
			if ((distance > 0) && (distance < PERCEPTION_RADIUS_ALIGNMENT)) {
				sum_velocity += boids[i].Velocity;
				neighbors++;
			}
		}

		glm::vec3 force = glm::vec3(0.0f, 0.0f, 0.0f);
		if (neighbors > 0) {
			force = sum_velocity / static_cast<float>(neighbors);
		}

		// Limit Force
		this->LimitForce(force, MAX_FORCE_MAGNITUDE);

		return force;
	}

	glm::vec3 Separation(std::vector<Boid> boids, float atten) {
		// Separation
		unsigned int neighbors = 0;
		glm::vec3 sum_pushback_force = glm::vec3(0.0f);

		for (int i = 0; i < boids.size(); i++) {
			float distance = glm::distance(this->Position, boids[i].Position);
			if ((distance > 0) && (distance < PERCEPTION_RADIUS_ALIGNMENT)) {
				glm::vec3 diff = this->Position - boids[i].Position;
				diff = glm::normalize(diff) / distance;
				sum_pushback_force += diff;
				neighbors++;
			}
		}

		glm::vec3 force = glm::vec3(0.0f, 0.0f, 0.0f);
		if (neighbors > 0) {
			force = sum_pushback_force / static_cast<float>(neighbors);
		}

		if (glm::length(force) > 0) {
			this->SetMagnitude(force, MAX_SPEED);
			force -= this->Velocity;

			// Limit Force
			this->LimitForce(force, MAX_FORCE_MAGNITUDE);
		}
		
		return force;
	}

	glm::vec3 Edges() {
		float distance = glm::distance(this->Position, glm::vec3(0.0f, 0.0f, 0.0f));
		float weight_factor = 1 / M_PI * atan((distance - WEIGHT_FACTOR_RADIUS_TOCENTER_FORCE) / 4.0f) + 0.5f;
		glm::vec3 force = -this->Position;
		force = glm::normalize(force);
		this->LimitForce(force, MAX_FORCE_MAGNITUDE * 8);
		force *= weight_factor;

		return force;
	}

	void flock(std::vector<Boid> boids, float s_atten, float a_atten, float c_atten) {
		unsigned int neighbors = 0;
		this->Acceleration *= 0;

		glm::vec3 avg_pushback_force = glm::vec3(0.0f);
		glm::vec3 avg_velocity = glm::vec3(0.0f);
		glm::vec3 avg_position = glm::vec3(0.0f);

		for (unsigned int i = 0; i < boids.size(); i++) {
			float distance = glm::distance(this->Position, boids[i].Position);
			if (distance > 0 && distance < this->PerceptionRadius) {
				// Separation
				glm::vec3 diff = this->Position - boids[i].Position;
				diff = glm::normalize(diff) / distance;
				avg_pushback_force += diff;

				// Alignment
				avg_velocity += boids[i].Velocity;
				
				// Cohesion
				avg_position += boids[i].Position;

				neighbors++;
			}
		}

		if (neighbors > 0) {
			avg_pushback_force /= neighbors;
			avg_pushback_force = this->SetMagnitude(avg_pushback_force, MAX_SPEED);
			avg_pushback_force -= this->Velocity;
			avg_pushback_force = this->LimitForce(avg_pushback_force, MAX_FORCE_MAGNITUDE);

			avg_velocity /= neighbors;
			avg_velocity = this->SetMagnitude(avg_velocity, MAX_SPEED);
			avg_velocity -= this->Velocity;
			avg_velocity = this->LimitForce(avg_velocity, MAX_FORCE_MAGNITUDE);

			avg_position /= neighbors;
			avg_position -= this->Position;
			avg_position = this->SetMagnitude(avg_position, MAX_SPEED);
			avg_position -= this->Velocity;
			avg_position = this->LimitForce(avg_position, MAX_FORCE_MAGNITUDE);
		}
		
		glm::vec3 separation = avg_pushback_force * s_atten;
		glm::vec3 alignment = avg_velocity * a_atten;
		glm::vec3 cohesion = avg_position * c_atten;
		
		this->Acceleration = separation + alignment + cohesion;
	}

	// Getter
	glm::mat4 getModel() const { return this->Model; }
	glm::vec3 getPosition() const { return this->Position; }
	glm::vec3 getVelocity() const { return this->Velocity; }
	glm::vec3 getAcceleration() const { return this->Acceleration; }

	float getSize() const { return this->getSize(); }

	// Setter
	void setModel(glm::mat4 model) { this->Model = model; }
	
private:
	glm::mat4 Model;
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec3 Acceleration;

	float PerceptionRadius;
	
	glm::vec3 LimitForce(glm::vec3 vector, float number) {
		glm::vec3 result = vector;
		if(glm::length(vector) > number) {
			result = glm::normalize(vector) * number;
		}
		return result;
	}

	glm::vec3 SetMagnitude(glm::vec3 vector, float number) {
		glm::vec3 result = glm::normalize(vector) * number;
		return result;
	}
};