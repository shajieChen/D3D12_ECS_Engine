#include "CCamera.h"
#include <Windows.h>
#include <algorithm>

XMFLOAT3 Camera::GetPosition()
{
	return position;
}

XMFLOAT3 Camera::GetDirection()
{
	XMVECTOR dir = XMVectorSet(direction.x, direction.y, direction.z, 0);
	auto rotQuaternion = XMQuaternionRotationRollPitchYaw(rotationX, rotationY, 0);
	dir = XMVector3Rotate(dir, rotQuaternion);
	XMStoreFloat3(&tdirection, dir);
	return tdirection;
}

void Camera::RotateX(float x)
{
	rotationX += x;
}

void Camera::RotateY(float y)
{
	rotationY += y;
}

XMFLOAT4X4 Camera::GetViewMatrix()
{
	auto rotQuaternion = XMQuaternionRotationRollPitchYaw(rotationX, rotationY, 0);
	XMVECTOR pos = XMVectorSet(position.x, position.y, position.z, 0);
	XMVECTOR dir = XMVectorSet(direction.x, direction.y, direction.z, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	dir = XMVector3Rotate(dir, rotQuaternion);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // 当前相机的位置
		dir,     // 当前相加的朝向
		up);     // 相机向上方向
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));
	return viewMatrix;
}

XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projectionMatrix;
}

void Camera::SetProjectionMatrix(float width, float height)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * XM_PI,
		width / height,
		0.1f,
		1000.0f);
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));
}

void Camera::Update(float deltaTime)
{
	float speed = 10.f;
	XMVECTOR pos = XMVectorSet(position.x, position.y, position.z, 0);
	XMVECTOR dir = XMVectorSet(direction.x, direction.y, direction.z, 0);
	auto rotQuaternion = XMQuaternionRotationRollPitchYaw(rotationX, rotationY, 0);
	dir = XMVector3Rotate(dir, rotQuaternion);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0); // Y is up!

	if (GetAsyncKeyState('W') & 0x8000)
	{
		pos = pos + dir * speed * deltaTime;;
	}

	if (GetAsyncKeyState('S') & 0x8000)
	{
		pos = pos - dir * speed * deltaTime;;
	}

	if (GetAsyncKeyState('A') & 0x8000)
	{
		auto leftDir = XMVector3Cross(dir, up);
		pos = pos + leftDir * speed * deltaTime;;
	}

	if (GetAsyncKeyState('D') & 0x8000)
	{
		auto rightDir = XMVector3Cross(-dir, up);
		pos = pos + rightDir * speed * deltaTime;;
	}
	XMStoreFloat3(&position, pos);
}

Camera::Camera(float width, float height)
{
	position = XMFLOAT3(0.f, 3.f, -15.f);
	direction = XMFLOAT3(0.f, 0.f, 1.f);
	rotationX = rotationY = 0.f;

	XMVECTOR pos = XMVectorSet(0, 0, -25, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     //当前相机的位置
		dir,     //当前相机的朝向
		up);     //相机向上看的位置
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V));

	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * XM_PI,		// FOV大小
		width / height,		// 宽高比
		0.1f,						// 近裁剪平面
		300.0f);					// 远裁剪平面
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P));
}

Camera::~Camera()
{
}

XMFLOAT3 Camera::GetUp()
{
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	auto rotQuaternion = XMQuaternionRotationRollPitchYaw(rotationX, rotationY, 0);
	up = XMVector3Rotate(up, rotQuaternion);
	XMStoreFloat3(&currentUp, up);
	return currentUp;
}