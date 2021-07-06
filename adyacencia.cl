#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void adyacencia(global const double* X_0, global const double* Y_0, global const int* particulas, global const int* lado, global int* adyacencia)
		{
			double r;
			double distancias[9];
			int idx = get_global_id(0);
			int particula_0 = idx % (*particulas);
			int particula_1 = idx / (*particulas);

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

			r = distancias[index];

			adyacencia[idx] = (r <= 1.1);
		}