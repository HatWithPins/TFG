void melleOpenCL(int particulas, int lado, double mason, double deltaT, double t, int repeticion) {
	//Esta función recibe como argumentos el número de partículas, el lado de la caja, numero de Mason, el intervalo de tiempos y el tiempo total..

	double campo[2] = { 1.0, 0.0 };
	double pi = 3.14159265359;
	double tiempo = 0.0;
	double deltaT_original = deltaT;
	double paso = 2 * pi / (mason * 360);
	int vueltas = ceil(t * mason / (2 * pi));
	int vuelta = 0;
	int contador = 0;
	int contador_vueltas = 0;
	double tramo = 0;
	int* distancias_validas = new int[particulas * particulas];
	int* desplazamientos_validos = new int[particulas];
	bool retroceso = false;

	Analisis analisis_estatico(mason, particulas, lado);

	Caja caja;
	caja.crearCaja(particulas, lado);
	caja.escribirPosiciones(0, mason, 0);

	//Aquí empieza OpenCL.
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if (all_platforms.size() == 0) {
		std::cout << " No se encontró ninguna plataforma. Comprueba la instalación de OpenCL.\n";
		exit(1);
	}

	//Buscando dispositivos con OpenCL.
	cl::Platform platform = all_platforms[0];
	//std::cout << "Usando la platforma: " << platform.getInfo<CL_PLATFORM_NAME>() << "\n";
	std::vector<cl::Device> all_devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &all_devices);
	if (all_devices.size() == 0) {
		std::cout << " No se encontraron dispositivos. Comprueba la instalación de OpenCL.\n";
		exit(1);
	}

	//Seleccionando la tarjeta gráfica.
	cl::Device device = all_devices[0];
	//std::cout << "Usando dispositivo: " << device.getInfo<CL_DEVICE_NAME>() << "\n";

	//Creando el contexto de OpenCL y creando la cola de trabajo.
	cl::Context contexto({ device });
	cl::CommandQueue cola = cl::CommandQueue(contexto, device);

	//Creando los buffers, la memoria en la gráfica.
	cl::Buffer bufferX_0 = cl::Buffer(contexto, CL_MEM_READ_ONLY, particulas * sizeof(double));
	cl::Buffer bufferY_0 = cl::Buffer(contexto, CL_MEM_READ_ONLY, particulas * sizeof(double));
	cl::Buffer bufferParticulas = cl::Buffer(contexto, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferLado = cl::Buffer(contexto, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferCampo = cl::Buffer(contexto, CL_MEM_READ_ONLY, 2 * sizeof(double));
	cl::Buffer bufferDeltaT = cl::Buffer(contexto, CL_MEM_READ_ONLY, sizeof(double));
	cl::Buffer bufferX_1 = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * sizeof(double));
	cl::Buffer bufferY_1 = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * sizeof(double));
	cl::Buffer bufferFuerzasX = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * particulas * sizeof(double));
	cl::Buffer bufferFuerzasY = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * particulas * sizeof(double));
	cl::Buffer bufferDistanciasValidas = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * particulas * sizeof(int));
	cl::Buffer bufferDesplazamientosValidos = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * sizeof(int));

	//Leer el fichero con el código.
	std::ifstream ficheroFuerzas("fuerzas.cl");
	std::string codigoFuerzas(std::istreambuf_iterator<char>(ficheroFuerzas), (std::istreambuf_iterator<char>()));
	cl::Program::Sources fuenteFuerzas;
	fuenteFuerzas.push_back({ codigoFuerzas.c_str(), codigoFuerzas.length() });

	std::ifstream ficheroSuma("suma.cl");
	std::string codigoSuma(std::istreambuf_iterator<char>(ficheroSuma), (std::istreambuf_iterator<char>()));
	cl::Program::Sources fuenteSuma;
	fuenteSuma.push_back({ codigoSuma.c_str(), codigoSuma.length() });

	std::ifstream ficheroDistancias("distancias.cl");
	std::string codigoDistancias(std::istreambuf_iterator<char>(ficheroDistancias), (std::istreambuf_iterator<char>()));
	cl::Program::Sources fuenteDistancias;
	fuenteDistancias.push_back({ codigoDistancias.c_str(), codigoDistancias.length() });

	//Crear los programa.
	cl::Program programaFuerzas = cl::Program(contexto, fuenteFuerzas);
	if (programaFuerzas.build({ device }) != CL_SUCCESS) {
		std::string problema = programaFuerzas.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		std::cout << " Error building: " << programaFuerzas.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
		exit(1);
	}

	cl::Program programaSuma = cl::Program(contexto, fuenteSuma);
	if (programaSuma.build({ device }) != CL_SUCCESS) {
		std::string problema = programaSuma.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		std::cout << " Error building: " << programaSuma.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
		exit(1);
	}

	cl::Program programaDistancias = cl::Program(contexto, fuenteDistancias);
	if (programaDistancias.build({ device }) != CL_SUCCESS) {
		std::string problema = programaDistancias.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		std::cout << " Error building: " << programaDistancias.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
		exit(1);
	}

	//Crear el kernel.
	cl::Kernel fuerzas_kernel(programaFuerzas, "fuerzas");
	cl::Kernel suma_kernel(programaSuma, "suma");
	cl::Kernel distancias_kernel(programaDistancias, "distancias");

	//Pasarle los argumentos al kernel.
	fuerzas_kernel.setArg(0, bufferX_0);
	fuerzas_kernel.setArg(1, bufferY_0);
	fuerzas_kernel.setArg(2, bufferLado);
	fuerzas_kernel.setArg(3, bufferParticulas);
	fuerzas_kernel.setArg(4, bufferCampo);
	fuerzas_kernel.setArg(5, bufferFuerzasX);
	fuerzas_kernel.setArg(6, bufferFuerzasY);

	suma_kernel.setArg(0, bufferX_0);
	suma_kernel.setArg(1, bufferY_0);
	suma_kernel.setArg(2, bufferLado);
	suma_kernel.setArg(3, bufferParticulas);
	suma_kernel.setArg(4, bufferDeltaT);
	suma_kernel.setArg(5, bufferFuerzasX);
	suma_kernel.setArg(6, bufferFuerzasY);
	suma_kernel.setArg(7, bufferDesplazamientosValidos);
	suma_kernel.setArg(8, bufferX_1);
	suma_kernel.setArg(9, bufferY_1);

	distancias_kernel.setArg(0, bufferX_0);
	distancias_kernel.setArg(1, bufferY_0);
	distancias_kernel.setArg(2, bufferLado);
	distancias_kernel.setArg(3, bufferParticulas);
	distancias_kernel.setArg(4, bufferDistanciasValidas);

	//Definir variables de ejecución de los kernels.
	cl::NDRange global_long(particulas * particulas);
	cl::NDRange global_short(particulas);
	cl::NDRange local(200);

	//Preparar las variables que contendrán las posiciones.
	std::vector<double> extraerX = caja.returnX();
	double* x_0 = extraerX.data();
	double* x_1 = new double[particulas];
	std::vector<double> extraerY = caja.returnY();
	double* y_0 = extraerY.data();
	double* y_1 = new double[particulas];

	for (int i = 0; i < particulas; i++) {
		x_1[i] = x_0[i];
		y_1[i] = y_0[i];
	}

	//Escribir los buffers.	
	cola.enqueueWriteBuffer(bufferParticulas, CL_TRUE, 0, sizeof(int), &particulas);
	cola.enqueueWriteBuffer(bufferLado, CL_TRUE, 0, sizeof(int), &lado);
	cola.enqueueWriteBuffer(bufferX_0, CL_TRUE, 0, particulas * sizeof(double), x_0);
	cola.enqueueWriteBuffer(bufferY_0, CL_TRUE, 0, particulas * sizeof(double), y_0);
	
	while (vuelta < vueltas) {
		//Verificar distancias mínimas.
		cola.enqueueNDRangeKernel(distancias_kernel, cl::NullRange, global_long, cl::NullRange);
		cola.enqueueReadBuffer(bufferDistanciasValidas, CL_TRUE, 0, particulas * particulas * sizeof(int), distancias_validas);

		for (int i = 0; i < particulas * particulas; i++) {
			if (distancias_validas[i] == 0) {
				retroceso = true;
				break;
			}
		}

		//Si la iteración no es válidad, se retrocede y modifican los parámetros.
		if (retroceso) {
			deltaT = deltaT / 2;

			for (int i = 0; i < particulas; i++) {
				x_1[i] = x_0[i];
				y_1[i] = y_0[i];
			}

			retroceso = false;
		}

		//Calculamos las fuerzas.
		cola.enqueueWriteBuffer(bufferX_0, CL_TRUE, 0, particulas * sizeof(double), x_0);
		cola.enqueueWriteBuffer(bufferY_0, CL_TRUE, 0, particulas * sizeof(double), y_0);
		cola.enqueueWriteBuffer(bufferCampo, CL_TRUE, 0, 2 * sizeof(double), &campo);
		cola.enqueueWriteBuffer(bufferDeltaT, CL_TRUE, 0, sizeof(double), &deltaT);

		cola.enqueueNDRangeKernel(fuerzas_kernel, cl::NullRange, global_long, cl::NullRange);

		//Sumamos las fuerzas.
		cola.enqueueNDRangeKernel(suma_kernel, cl::NullRange, global_short, cl::NullRange);
		cola.enqueueReadBuffer(bufferDesplazamientosValidos, CL_TRUE, 0, particulas * sizeof(int), desplazamientos_validos);
		cola.enqueueReadBuffer(bufferX_1, CL_TRUE, 0, particulas * sizeof(double), x_1);
		cola.enqueueReadBuffer(bufferY_1, CL_TRUE, 0, particulas * sizeof(double), y_1);

		//Comprobamos si el tamaño de los desplazamientos es válido.
		for (int i = 0; i < particulas; i++) {
			if (desplazamientos_validos[i] == 0) {
				retroceso = true;
				break;
			}
		}

		//Si la iteración no es válidad, se retrocede y modifican los parámetros.
		if (retroceso) {
			deltaT = deltaT / 2;

			for (int i = 0; i < particulas; i++) {
				x_1[i] = x_0[i];
				y_1[i] = y_0[i];
			}

			retroceso = false;
		}
		//En caso contrario, la simulación sigue a su siguiente estadío.
		else {

			for (int i = 0; i < particulas; i++) {
				x_0[i] = x_1[i];
				y_0[i] = y_1[i];
			}

			tiempo += deltaT;
			deltaT = deltaT_original;

			campo[0] = cos(mason * tiempo);
			campo[1] = sin(mason * tiempo);

			contador_vueltas++;
			if (contador_vueltas >= 1000 && repeticion == 0) {
				caja.setX(x_0);
				caja.setY(y_0);
				caja.escribirPosiciones(contador, mason, 0);
				contador_vueltas = 0;
				contador++;
			}
		}

		//Si se está en la última vuelta, ir tomando los datos para su análisis.
		vuelta = floor(tiempo * mason / (2 * pi));

		if (vuelta == vueltas - 1) {
			tramo += deltaT;

			if (tramo > paso) {
				analisis_estatico.pre_analisis(x_0, y_0);
				tramo = 0;
			}
		}
	}

	if (repeticion == 0) {
		caja.setX(x_0);
		caja.setY(y_0);
		caja.escribirPosiciones(contador, mason, 0);
	}
	//Escribir el fichero.
	analisis_estatico.analisis();
	analisis_estatico.escribir_analisis(repeticion);
}

void moctezumaOpenCL(int particulas, int lado, double mason, double deltaT, double t, int repeticion) {
	//Esta función recibe como argumentos el número de partículas, el lado de la caja, numero de Mason, el intervalo de tiempos y el tiempo total..
	std::cout << "Iniciando simulacion estatica para mason = " + std::to_string(mason) + "\n";

	double campo[2] = { 0.0, 1.0 };
	double pi = 3.14159265359;
	double tiempo = 0.0;
	double t_estatico = 1000;
	double deltaT_original = deltaT;
	double paso = 2 * pi / (2 * 360);
	int vueltas = ceil(t_estatico * 2 / (2 * pi));
	int vuelta = 0;
	int contador = 0;
	int contador_vueltas = 0;
	double tramo = 0;
	int* distancias_validas = new int[particulas * particulas];
	int* desplazamientos_validos = new int[particulas];
	bool retroceso = false;

	Analisis analisis_estatico(mason, particulas, lado);
	Analisis analisis_perturbado(mason, particulas, lado);

	Caja caja;
	caja.crearCaja(particulas, lado);
	caja.escribirPosiciones(0, mason, 0);

	//Aquí empieza OpenCL.
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if (all_platforms.size() == 0) {
		std::cout << " No se encontró ninguna plataforma. Comprueba la instalación de OpenCL.\n";
		exit(1);
	}

	//Buscando dispositivos con OpenCL.
	cl::Platform platform = all_platforms[0];
	//std::cout << "Usando la platforma: " << platform.getInfo<CL_PLATFORM_NAME>() << "\n";
	std::vector<cl::Device> all_devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &all_devices);
	if (all_devices.size() == 0) {
		std::cout << " No se encontraron dispositivos. Comprueba la instalación de OpenCL.\n";
		exit(1);
	}

	//Seleccionando la tarjeta gráfica.
	cl::Device device = all_devices[0];
	//std::cout << "Usando dispositivo: " << device.getInfo<CL_DEVICE_NAME>() << "\n";

	//Creando el contexto de OpenCL y creando la cola de trabajo.
	cl::Context contexto({ device });
	cl::CommandQueue cola = cl::CommandQueue(contexto, device);

	//Creando los buffers, la memoria en la gráfica.
	cl::Buffer bufferX_0 = cl::Buffer(contexto, CL_MEM_READ_ONLY, particulas * sizeof(double));
	cl::Buffer bufferY_0 = cl::Buffer(contexto, CL_MEM_READ_ONLY, particulas * sizeof(double));
	cl::Buffer bufferParticulas = cl::Buffer(contexto, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferLado = cl::Buffer(contexto, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferCampo = cl::Buffer(contexto, CL_MEM_READ_ONLY, 2 * sizeof(double));
	cl::Buffer bufferDeltaT = cl::Buffer(contexto, CL_MEM_READ_ONLY, sizeof(double));
	cl::Buffer bufferX_1 = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * sizeof(double));
	cl::Buffer bufferY_1 = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * sizeof(double));
	cl::Buffer bufferFuerzasX = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * particulas * sizeof(double));
	cl::Buffer bufferFuerzasY = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * particulas * sizeof(double));
	cl::Buffer bufferDistanciasValidas = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * particulas * sizeof(int));
	cl::Buffer bufferDesplazamientosValidos = cl::Buffer(contexto, CL_MEM_READ_WRITE, particulas * sizeof(int));

	//Leer el fichero con el código.
	std::ifstream ficheroFuerzas("fuerzas.cl");
	std::string codigoFuerzas(std::istreambuf_iterator<char>(ficheroFuerzas), (std::istreambuf_iterator<char>()));
	cl::Program::Sources fuenteFuerzas;
	fuenteFuerzas.push_back({ codigoFuerzas.c_str(), codigoFuerzas.length() });

	std::ifstream ficheroSuma("suma.cl");
	std::string codigoSuma(std::istreambuf_iterator<char>(ficheroSuma), (std::istreambuf_iterator<char>()));
	cl::Program::Sources fuenteSuma;
	fuenteSuma.push_back({ codigoSuma.c_str(), codigoSuma.length() });

	std::ifstream ficheroDistancias("distancias.cl");
	std::string codigoDistancias(std::istreambuf_iterator<char>(ficheroDistancias), (std::istreambuf_iterator<char>()));
	cl::Program::Sources fuenteDistancias;
	fuenteDistancias.push_back({ codigoDistancias.c_str(), codigoDistancias.length() });

	//Crear los programa.
	cl::Program programaFuerzas = cl::Program(contexto, fuenteFuerzas);
	if (programaFuerzas.build({ device }) != CL_SUCCESS) {
		std::string problema = programaFuerzas.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		std::cout << " Error building: " << programaFuerzas.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
		exit(1);
	}

	cl::Program programaSuma = cl::Program(contexto, fuenteSuma);
	if (programaSuma.build({ device }) != CL_SUCCESS) {
		std::string problema = programaSuma.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		std::cout << " Error building: " << programaSuma.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
		exit(1);
	}

	cl::Program programaDistancias = cl::Program(contexto, fuenteDistancias);
	if (programaDistancias.build({ device }) != CL_SUCCESS) {
		std::string problema = programaDistancias.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
		std::cout << " Error building: " << programaDistancias.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
		exit(1);
	}

	//Crear el kernel.
	cl::Kernel fuerzas_kernel(programaFuerzas, "fuerzas");
	cl::Kernel suma_kernel(programaSuma, "suma");
	cl::Kernel distancias_kernel(programaDistancias, "distancias");

	//Pasarle los argumentos al kernel.
	fuerzas_kernel.setArg(0, bufferX_0);
	fuerzas_kernel.setArg(1, bufferY_0);
	fuerzas_kernel.setArg(2, bufferLado);
	fuerzas_kernel.setArg(3, bufferParticulas);
	fuerzas_kernel.setArg(4, bufferCampo);
	fuerzas_kernel.setArg(5, bufferFuerzasX);
	fuerzas_kernel.setArg(6, bufferFuerzasY);

	suma_kernel.setArg(0, bufferX_0);
	suma_kernel.setArg(1, bufferY_0);
	suma_kernel.setArg(2, bufferLado);
	suma_kernel.setArg(3, bufferParticulas);
	suma_kernel.setArg(4, bufferDeltaT);
	suma_kernel.setArg(5, bufferFuerzasX);
	suma_kernel.setArg(6, bufferFuerzasY);
	suma_kernel.setArg(7, bufferDesplazamientosValidos);
	suma_kernel.setArg(8, bufferX_1);
	suma_kernel.setArg(9, bufferY_1);

	distancias_kernel.setArg(0, bufferX_0);
	distancias_kernel.setArg(1, bufferY_0);
	distancias_kernel.setArg(2, bufferLado);
	distancias_kernel.setArg(3, bufferParticulas);
	distancias_kernel.setArg(4, bufferDistanciasValidas);

	//Definir variables de ejecución de los kernels.
	cl::NDRange global_long(particulas * particulas);
	cl::NDRange global_short(particulas);
	cl::NDRange local(200);

	//Preparar las variables que contendrán las posiciones.
	std::vector<double> extraerX = caja.returnX();
	double* x_0 = extraerX.data();
	double* x_1 = new double[particulas];
	std::vector<double> extraerY = caja.returnY();
	double* y_0 = extraerY.data();
	double* y_1 = new double[particulas];

	for (int i = 0; i < particulas; i++) {
		x_1[i] = x_0[i];
		y_1[i] = y_0[i];
	}

	//Escribir los buffers.	
	cola.enqueueWriteBuffer(bufferParticulas, CL_TRUE, 0, sizeof(int), &particulas);
	cola.enqueueWriteBuffer(bufferLado, CL_TRUE, 0, sizeof(int), &lado);
	cola.enqueueWriteBuffer(bufferX_0, CL_TRUE, 0, particulas * sizeof(double), x_0);
	cola.enqueueWriteBuffer(bufferY_0, CL_TRUE, 0, particulas * sizeof(double), y_0);

	while (vuelta < vueltas) {
		//Verificar distancias mínimas.
		cola.enqueueNDRangeKernel(distancias_kernel, cl::NullRange, global_long, cl::NullRange);
		cola.enqueueReadBuffer(bufferDistanciasValidas, CL_TRUE, 0, particulas * particulas * sizeof(int), distancias_validas);

		for (int i = 0; i < particulas * particulas; i++) {
			if (distancias_validas[i] == 0) {
				retroceso = true;
				break;
			}
		}

		//Si la iteración no es válidad, se retrocede y modifican los parámetros.
		if (retroceso) {
			deltaT = deltaT / 2;

			for (int i = 0; i < particulas; i++) {
				x_1[i] = x_0[i];
				y_1[i] = y_0[i];
			}

			retroceso = false;
		}

		//Calculamos las fuerzas.
		cola.enqueueWriteBuffer(bufferX_0, CL_TRUE, 0, particulas * sizeof(double), x_0);
		cola.enqueueWriteBuffer(bufferY_0, CL_TRUE, 0, particulas * sizeof(double), y_0);
		cola.enqueueWriteBuffer(bufferCampo, CL_TRUE, 0, 2 * sizeof(double), &campo);
		cola.enqueueWriteBuffer(bufferDeltaT, CL_TRUE, 0, sizeof(double), &deltaT);

		cola.enqueueNDRangeKernel(fuerzas_kernel, cl::NullRange, global_long, cl::NullRange);

		//Sumamos las fuerzas.
		cola.enqueueNDRangeKernel(suma_kernel, cl::NullRange, global_short, cl::NullRange);
		cola.enqueueReadBuffer(bufferDesplazamientosValidos, CL_TRUE, 0, particulas * sizeof(int), desplazamientos_validos);
		cola.enqueueReadBuffer(bufferX_1, CL_TRUE, 0, particulas * sizeof(double), x_1);
		cola.enqueueReadBuffer(bufferY_1, CL_TRUE, 0, particulas * sizeof(double), y_1);

		//Comprobamos si el tamaño de los desplazamientos es válido.
		for (int i = 0; i < particulas; i++) {
			if (desplazamientos_validos[i] == 0) {
				retroceso = true;
				break;
			}
		}

		//Si la iteración no es válidad, se retrocede y modifican los parámetros.
		if (retroceso) {
			deltaT = deltaT / 2;

			for (int i = 0; i < particulas; i++) {
				x_1[i] = x_0[i];
				y_1[i] = y_0[i];
			}

			retroceso = false;
		}
		//En caso contrario, la simulación sigue a su siguiente estadío.
		else {

			for (int i = 0; i < particulas; i++) {
				x_0[i] = x_1[i];
				y_0[i] = y_1[i];
			}

			tiempo += deltaT;
			deltaT = deltaT_original;

			contador_vueltas++;
			if (contador_vueltas >= 1000 && repeticion == 0) {
				caja.setX(x_0);
				caja.setY(y_0);
				caja.escribirPosiciones(contador, mason, 0);
				contador_vueltas = 0;
				contador++;
			}
		}

		//Si se está en la última vuelta, ir tomando los datos para su análisis.
		vuelta = floor(tiempo * 2 / (2 * pi));

		if (vuelta == vueltas - 1) {
			tramo += deltaT;

			if (tramo > paso) {
				analisis_estatico.pre_analisis(x_0, y_0);
				tramo = 0;
			}
		}
	}

	if (repeticion == 0) {
		caja.setX(x_0);
		caja.setY(y_0);
		caja.escribirPosiciones(contador, mason, 0);
	}
	//Escribir el fichero.
	analisis_estatico.analisis();
	analisis_estatico.escribir_analisis(repeticion);

	//----------------------------------------------------------------------------------------------------------------------------
	//Version perturbada
	std::cout << "Iniciando simulacion perturbada para mason = " + std::to_string(mason) + "\n";

	tiempo = 0.0;
	vuelta = 0;
	contador_vueltas = 0;
	tramo = 0;
	retroceso = false;
	deltaT = deltaT_original;
	paso = 2 * pi / (mason * 360);
	vueltas = ceil(t * mason / (2 * pi));

	for (int i = 0; i < particulas; i++) {
		x_1[i] = x_0[i];
		y_1[i] = y_0[i];
	}

	caja.setX(x_0);
	caja.setY(y_0);
	caja.escribirPosiciones(contador, mason, 0);

	//Escribir los buffers.	
	cola.enqueueWriteBuffer(bufferX_0, CL_TRUE, 0, particulas * sizeof(double), x_0);
	cola.enqueueWriteBuffer(bufferY_0, CL_TRUE, 0, particulas * sizeof(double), y_0);

	while (vuelta < vueltas) {
		//Verificar distancias mínimas.
		cola.enqueueNDRangeKernel(distancias_kernel, cl::NullRange, global_long, cl::NullRange);
		cola.enqueueReadBuffer(bufferDistanciasValidas, CL_TRUE, 0, particulas * particulas * sizeof(int), distancias_validas);

		for (int i = 0; i < particulas * particulas; i++) {
			if (distancias_validas[i] == 0) {
				retroceso = true;
				break;
			}
		}

		//Si la iteración no es válidad, se retrocede y modifican los parámetros.
		if (retroceso) {
			deltaT = deltaT / 2;

			for (int i = 0; i < particulas; i++) {
				x_1[i] = x_0[i];
				y_1[i] = y_0[i];
			}

			retroceso = false;
		}

		//Calculamos las fuerzas.
		cola.enqueueWriteBuffer(bufferX_0, CL_TRUE, 0, particulas * sizeof(double), x_0);
		cola.enqueueWriteBuffer(bufferY_0, CL_TRUE, 0, particulas * sizeof(double), y_0);
		cola.enqueueWriteBuffer(bufferCampo, CL_TRUE, 0, 2 * sizeof(double), &campo);
		cola.enqueueWriteBuffer(bufferDeltaT, CL_TRUE, 0, sizeof(double), &deltaT);

		cola.enqueueNDRangeKernel(fuerzas_kernel, cl::NullRange, global_long, cl::NullRange);

		//Sumamos las fuerzas.
		cola.enqueueNDRangeKernel(suma_kernel, cl::NullRange, global_short, cl::NullRange);
		cola.enqueueReadBuffer(bufferDesplazamientosValidos, CL_TRUE, 0, particulas * sizeof(int), desplazamientos_validos);
		cola.enqueueReadBuffer(bufferX_1, CL_TRUE, 0, particulas * sizeof(double), x_1);
		cola.enqueueReadBuffer(bufferY_1, CL_TRUE, 0, particulas * sizeof(double), y_1);

		//Comprobamos si el tamaño de los desplazamientos es válido.
		for (int i = 0; i < particulas; i++) {
			if (desplazamientos_validos[i] == 0) {
				retroceso = true;
				break;
			}
		}

		//Si la iteración no es válidad, se retrocede y modifican los parámetros.
		if (retroceso) {
			deltaT = deltaT / 2;

			for (int i = 0; i < particulas; i++) {
				x_1[i] = x_0[i];
				y_1[i] = y_0[i];
			}

			retroceso = false;
		}
		//En caso contrario, la simulación sigue a su siguiente estadío.
		else {

			for (int i = 0; i < particulas; i++) {
				x_0[i] = x_1[i];
				y_0[i] = y_1[i];
			}

			tiempo += deltaT;
			deltaT = deltaT_original;

			campo[1] = 1 / sqrt(sin(mason * tiempo) * sin(mason * tiempo) + 1);
			campo[0] = sin(mason * tiempo) * campo[1];

			contador_vueltas++;
			if (contador_vueltas >= 1000 && repeticion == 0) {
				caja.setX(x_0);
				caja.setY(y_0);
				caja.escribirPosiciones(contador, mason, 0);
				contador_vueltas = 0;
				contador++;
			}
		}

		//Si se está en la última vuelta, ir tomando los datos para su análisis.
		vuelta = floor(tiempo * mason / (2 * pi));

		if (vuelta == vueltas - 1) {
			tramo += deltaT;

			if (tramo > paso) {
				analisis_perturbado.pre_analisis(x_0, y_0);
				tramo = 0;
			}
		}
	}

	if (repeticion == 0) {
		caja.setX(x_0);
		caja.setY(y_0);
		caja.escribirPosiciones(contador, mason, 0);
	}
	//Escribir el fichero.
	analisis_perturbado.analisis();
	analisis_perturbado.escribir_analisis(repeticion + 10);
}