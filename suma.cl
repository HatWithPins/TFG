#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void suma (global const double* X_0, global const double* Y_0, global const int* lado, global const int* particulas, global const double* deltaT, global double* fuerzas_X, global double* fuerzas_Y, global int* valido, global double* X_1, global double* Y_1) {
			double r;
			int idx = get_global_id(0);
			int restoX;
			int restoY;

			X_1[idx] = 0;
			Y_1[idx] = 0;

			for (int i = 0; i < (*particulas); i++) {
				X_1[idx] += fuerzas_X[idx * (*particulas) + i];
				Y_1[idx] += fuerzas_Y[idx * (*particulas) + i];
			}

			X_1[idx] = (*deltaT) * X_1[idx];
			Y_1[idx] = (*deltaT) * Y_1[idx];

			r = (sqrt(X_1[idx] * X_1[idx] + Y_1[idx] * Y_1[idx]));
			valido[idx] = (r <= 0.03);

			X_1[idx] = X_0[idx] +  X_1[idx];
			Y_1[idx] = Y_0[idx] +  Y_1[idx];

			restoX = X_1[idx] / (*lado);
			X_1[idx] = X_1[idx] - restoX * (*lado) + (*lado) * (X_1[idx] < 0);
			restoY = Y_1[idx] / (*lado);
			Y_1[idx] = Y_1[idx] - restoY * (*lado) + (*lado) * (Y_1[idx] < 0);
}