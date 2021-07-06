#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void dinamica(global const float* X_0, global const float* Y_0, global const int* particulas, global const int* lado, global const float* campo, global const float* modulo, global float* X_1, global float* Y_1)
		{
			float pi = 3.14159265358979323846;
			float mu = 4 * pi * 0.0000000000001;
			float deltaT = 0.005;
			float factor = 12 / (pi * 0.25 * mu * pown((*modulo),2));
			float A = 2;
			float B = 10;
			float r;
			float escalar;
			float distancias[9];
			float x;
			float y;
			int idx = get_global_id(0);
			X_1[idx] = 0;
			Y_1[idx] = 0;
			int restoX;
			int restoY;
			for (int i = 0; i < *particulas; i++) {
				if (i != get_global_id(0)) {
					distancias[0] = (sqrt(pown(X_0[i] - X_0[idx],2) + pown(Y_0[i] - Y_0[idx],2)));
					distancias[1] = (sqrt(pown(X_0[i] - X_0[idx],2) + pown(Y_0[i] - Y_0[idx] - (*lado),2)));
					distancias[2] = (sqrt(pown(X_0[i] - X_0[idx] + (*lado),2) + pown(Y_0[i] - Y_0[idx] - (*lado),2)));
					distancias[3] = (sqrt(pown(X_0[i] - X_0[idx] + (*lado),2) + pown(Y_0[i] - Y_0[idx],2)));
					distancias[4] = (sqrt(pown(X_0[i] - X_0[idx] + (*lado),2) + pown(Y_0[i] - Y_0[idx] + (*lado),2)));
					distancias[5] = (sqrt(pown(X_0[i] - X_0[idx],2) + pown(Y_0[i] - Y_0[idx] + (*lado),2)));
					distancias[6] = (sqrt(pown(X_0[i] - X_0[idx] - (*lado),2) + pown(Y_0[i] - Y_0[idx] + (*lado),2)));
					distancias[7] = (sqrt(pown(X_0[i] - X_0[idx] - (*lado),2) + pown(Y_0[i] - Y_0[idx],2)));
					distancias[8] = (sqrt(pown(X_0[i] - X_0[idx] - (*lado),2) + pown(Y_0[i] - Y_0[idx] - (*lado),2)));
					int index = 0;
					for(int j = 0; j < 9; j++) {
						if(distancias[index] > distancias[j]){ index = j;}
					}
					x = (*lado) * (index == 2) + (*lado) * (index == 3) + (*lado) * (index == 4) - (*lado) * (index == 6) - (*lado) * (index == 7) - (*lado) * (index == 8) + X_0[i] - X_0[idx];
					y = (*lado) * (index == 4) + (*lado) * (index == 5) + (*lado) * (index == 6) - (*lado) * (index == 1) - (*lado) * (index == 2) - (*lado) * (index == 8) + Y_0[i] - Y_0[idx];
					r = distancias[index];
					escalar = campo[0] * x + campo[1] * y;
					X_1[idx] += 3 * mu * pown((*modulo), 2) / (4 * pi * pown(r, 4)) * ((1 - 5 * escalar) * x + 2 * escalar * campo[0]) + A * 3 * mu * pown((*modulo), 2) / (4 * pi) * exp(-B * r - 1) * x;
					Y_1[idx] += 3 * mu * pown((*modulo), 2) / (4 * pi * pown(r, 4)) * ((1 - 5 * escalar) * y + 2 * escalar * campo[1]) + A * 3 * mu * pown((*modulo), 2) / (4 * pi) * exp(-B * r - 1) * y;
				}
			}
			X_1[idx] = deltaT * factor * X_1[idx];
			Y_1[idx] = deltaT * factor * Y_1[idx];
			restoX = X_1[idx] / (*lado);
			X_1[idx] = X_1[idx] - restoX * (*lado) + (*lado) * (X_1[idx] < 0);
			restoY = Y_1[idx] / (*lado);
			Y_1[idx] = Y_1[idx] - restoY * (*lado) + (*lado) * (Y_1[idx] < 0);
		}