#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void fuerzas (global const double* X_0, global const double* Y_0, global const int* lado, global const int* particulas, global const double* campo, global double* fuerzas_X, global double* fuerzas_Y) {
			double r;
			double A = 2;
			double B = 10;
			double x;
			double y;
			double escalar;
			double distancias[9];
			int idx = get_global_id(0);
			int particula_0 = idx % (*particulas);
			int particula_1 = idx / (*particulas);

			if (particula_0 != particula_1) {
				distancias[0] = (sqrt(pown(X_0[particula_1] - X_0[particula_0],2) + pown(Y_0[particula_1] - Y_0[particula_0],2)));
				distancias[1] = (sqrt(pown(X_0[particula_1] - X_0[particula_0],2) + pown(Y_0[particula_1] - Y_0[particula_0] - (*lado),2)));
				distancias[2] = (sqrt(pown(X_0[particula_1] - X_0[particula_0] + (*lado),2) + pown(Y_0[particula_1] - Y_0[particula_0] - (*lado),2)));
				distancias[3] = (sqrt(pown(X_0[particula_1] - X_0[particula_0] + (*lado),2) + pown(Y_0[particula_1] - Y_0[particula_0],2)));
				distancias[4] = (sqrt(pown(X_0[particula_1] - X_0[particula_0] + (*lado),2) + pown(Y_0[particula_1] - Y_0[particula_0] + (*lado),2)));
				distancias[5] = (sqrt(pown(X_0[particula_1] - X_0[particula_0],2) + pown(Y_0[particula_1] - Y_0[particula_0] + (*lado),2)));
				distancias[6] = (sqrt(pown(X_0[particula_1] - X_0[particula_0] - (*lado),2) + pown(Y_0[particula_1] - Y_0[particula_0] + (*lado),2)));
				distancias[7] = (sqrt(pown(X_0[particula_1] - X_0[particula_0] - (*lado),2) + pown(Y_0[particula_1] - Y_0[particula_0],2)));
				distancias[8] = (sqrt(pown(X_0[particula_1] - X_0[particula_0] - (*lado),2) + pown(Y_0[particula_1] - Y_0[particula_0] - (*lado),2)));
				int index = 0;
				for(int j = 0; j < 9; j++) {
					if(distancias[index] > distancias[j]){ index = j;}
				}
				x = (*lado) * ((index == 2) + (index == 3) + (index == 4) - (index == 6) - (index == 7) - (index == 8)) + X_0[particula_1] - X_0[particula_0];
				y = (*lado) * ((index == 4) + (index == 5) + (index == 6) - (index == 1) - (index == 2) - (index == 8)) + Y_0[particula_1] - Y_0[particula_0];
				r = distancias[index];
				x = x / r;
				y = y / r;

				escalar = campo[0] * x + campo[1] * y;

				fuerzas_X[idx] = ((1 - 5 * pown(escalar, 2)) / (pown(r, 4)) + A * exp(-B * (r - 1))) * x + (2 * escalar * campo[0]) / (pown(r, 4));
				fuerzas_Y[idx] = ((1 - 5 * pown(escalar, 2)) / (pown(r, 4)) + A * exp(-B * (r - 1))) * y + (2 * escalar * campo[1]) / (pown(r, 4));
			}
			else {
				fuerzas_X[idx] = 0;
				fuerzas_Y[idx] = 0;
			}
}