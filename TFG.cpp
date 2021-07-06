#include "TFG.h"

using namespace std;
using namespace std::chrono;

void simulacion(int particulas, int lado, double mason, double deltaT, double tiempo, int repeticion) {
	auto start = high_resolution_clock::now();
	melleOpenCL(particulas, lado, mason, deltaT, tiempo, repeticion);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<seconds>(stop - start);

	cout << "Tiempo para ma = " << mason << ": "
		<< duration.count() << " segundos" << endl;
}

void hilos_moctezuma(int particulas, int lado, double mason, double deltaT, double tiempo, int repeticion) {
	auto start = high_resolution_clock::now();
	moctezumaOpenCL(particulas, lado, mason, deltaT, tiempo, repeticion);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<seconds>(stop - start);

	cout << "Tiempo para ma = " << mason << ": "
		<< duration.count() << " segundos" << endl;
}

int main()
{
	auto start = high_resolution_clock::now();
	int particulas = 400;
	//int lado = 140;
	int lado = 67;
	double mason[4] = { 0.001, 0.004, 1.0, 2.0 };
	int repeticiones = 5;
	double tiempo = 1000;
	double deltaT = 0.001;

	/*std::thread hilos_melle[5];
	
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < repeticiones; j++) {
			hilos_melle[j] = std::thread(simulacion, particulas, lado, mason[i], deltaT, tiempo, j);
		}
		for (int j = 0; j < repeticiones; j++) {
			hilos_melle[j].join();
		}
	}*/

	std::thread hilos[5];

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < repeticiones; j++) {
			hilos[j] = std::thread(hilos_moctezuma, particulas, lado, mason[i], deltaT, tiempo, j);
		}
		for (int j = 0; j < repeticiones; j++) {
			hilos[j].join();
		}
	}

	/*for (int i = 0; i < repeticiones; i++) {
		for (int j = 0; j < 4; j++) {
			auto comienzo = high_resolution_clock::now();
			melleOpenCL(particulas, lado, mason[j], deltaT, tiempo, i);
			auto fin = high_resolution_clock::now();
			auto duration = duration_cast<milliseconds>(fin - comienzo);

			cout << "Tiempo para ma = " << mason[j] << " y repeticion " << i + 1 << ": "
				<< duration.count() << " milisegundos" << endl;
		}
	}*/

	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<seconds>(stop - start);

	cout << "Tiempo total: "
		<< duration.count() << " segundos" << endl;

	return 0;
}
