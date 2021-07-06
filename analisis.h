class Analisis {
private:

	double m_mason; //Numero de mason.
	int m_iteracion; //Contador de las veces que he llamado a la funcion pre_analisis.
	int m_particulas; //Número de partículas.
	int m_lado; //Lado de la caja, un múltiplo del diámetro de las partículas.
	std::vector<std::vector<int>> cadenas; //Vector de dos dimensiones que contiene las cadenas.
	std::vector<std::vector<int>> tamannos; //Vector de dos dimensiones que contiene los tamannos de las cadenas.
	std::vector<std::vector<double>> linealidades; //Vector de dos dimensiones que contiene las linealidades de las cadenas.
	std::vector<std::vector<double>> medias; //Vector con las medias y desviaciones estandar.

	int* Adyacencia(double* x, double* y) {
		//Funcion para obtener la adyacencia entre cadenas

		int* adyacencia= new int[m_particulas * m_particulas];

		//Aquí empieza OpenCL.
		std::vector<cl::Platform> all_platforms;
		cl::Platform::get(&all_platforms);
		if (all_platforms.size() == 0) {
			std::cout << " No se encontró ninguna plataforma. Comprueba la instalación de OpenCL.\n";
			exit(1);
		}

		//Buscando dispositivos con OpenCL.
		cl::Platform platform = all_platforms[0];
		std::vector<cl::Device> all_devices;
		platform.getDevices(CL_DEVICE_TYPE_GPU, &all_devices);
		if (all_devices.size() == 0) {
			std::cout << " No se encontraron dispositivos. Comprueba la instalación de OpenCL.\n";
			exit(1);
		}

		//Seleccionando la tarjeta gráfica.
		cl::Device device = all_devices[0];

		//Creando el contexto de OpenCL y creando la cola de trabajo.
		cl::Context contexto({ device });
		cl::CommandQueue cola = cl::CommandQueue(contexto, device);

		//Creando los buffers, la memoria en la gráfica.
		cl::Buffer bufferX = cl::Buffer(contexto, CL_MEM_READ_ONLY, m_particulas * sizeof(double));
		cl::Buffer bufferY = cl::Buffer(contexto, CL_MEM_READ_ONLY, m_particulas * sizeof(double));
		cl::Buffer bufferParticulas = cl::Buffer(contexto, CL_MEM_READ_ONLY, sizeof(int));
		cl::Buffer bufferLado = cl::Buffer(contexto, CL_MEM_READ_ONLY, sizeof(int));
		cl::Buffer bufferAdyacencia = cl::Buffer(contexto, CL_MEM_READ_WRITE, m_particulas * m_particulas * sizeof(double));

		cola.enqueueWriteBuffer(bufferX, CL_TRUE, 0, m_particulas * sizeof(double), x);
		cola.enqueueWriteBuffer(bufferY, CL_TRUE, 0, m_particulas * sizeof(double), y);
		cola.enqueueWriteBuffer(bufferParticulas, CL_TRUE, 0, sizeof(int), &m_particulas);
		cola.enqueueWriteBuffer(bufferLado, CL_TRUE, 0, sizeof(int), &m_lado);

		//Leer el fichero con el código.
		std::ifstream ficheroFuente("adyacencia.cl");
		std::string codigoFuente(std::istreambuf_iterator<char>(ficheroFuente), (std::istreambuf_iterator<char>()));
		cl::Program::Sources fuente;
		fuente.push_back({ codigoFuente.c_str(), codigoFuente.length() });

		//Crear el programa.
		cl::Program programa = cl::Program(contexto, fuente);
		if (programa.build({ device }) != CL_SUCCESS) {
			std::string problema = programa.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
			std::cout << " Error building: " << programa.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
			exit(1);
		}

		//Crear el kernel.
		cl::Kernel adyacencia_kernel(programa, "adyacencia");

		//Pasarle los argumentos al kernel.
		adyacencia_kernel.setArg(0, bufferX);
		adyacencia_kernel.setArg(1, bufferY);
		adyacencia_kernel.setArg(2, bufferParticulas);
		adyacencia_kernel.setArg(3, bufferLado);
		adyacencia_kernel.setArg(4, bufferAdyacencia);

		//Correr el kernel.
		cl::NDRange global(m_particulas * m_particulas);
		cl::NDRange local(200);
		cola.enqueueNDRangeKernel(adyacencia_kernel, cl::NullRange, global, cl::NullRange);

		//Copiar los resultados en local.
		cola.enqueueReadBuffer(bufferAdyacencia, CL_TRUE, 0, m_particulas * m_particulas * sizeof(int), adyacencia);

		return adyacencia;
	}

	std::vector<int> BFS(int* adyacencia) {
		//Me devuelve un vector que me indica a que cadena pertenece cada particula.
		std::vector<int> cadena;
		std::vector<int> lista(1, 0);
		std::vector<int> visitado(m_particulas, 0);

		for (int i = 0; i < m_particulas; i++) {
			cadena.push_back(i);
		}

		bool visitados = false;
		int nodo = 0;

		while (visitados == false) {
			//Marco como visitado el nodo actual.
			visitado[lista[0]] = 1;

			//Compruebo cuales son adyacentes y no han sido visitados ya y no estan en la lista para añadirlos.
			for (int i = 0; i < m_particulas; i++) {
				if (adyacencia[m_particulas * lista[0] + i] == 1 && visitado[i] == 0 && !(std::find(lista.begin(), lista.end(), i) != lista.end())) {
					lista.push_back(i);
				}
			}

			//Añado el nodo a su respectiva cadena.
			cadena[lista[0]] = min(nodo, cadena[lista[0]]);

			lista.erase(lista.begin());

			//Compruebo si ya he visitado todos los nodos.
			if (std::find(visitado.begin(), visitado.end(), 0) == visitado.end()) {
				visitados = true;
			}
			else {
				//Compruebo si no quedan mas nodos que visitar en la cadena actual
				if (lista.empty() == true) {
					//Marco para visitar el siguiente nodo en la lista de visitados que no lo este ya.
					for (int i = 0; i < m_particulas; i++) {
						if (visitado[i] == 0) {
							lista.push_back(i);
							nodo = i;
							break;
						}
					}
				}
				//En caso contrario, procedo con el siguiente en la lista y con el bucle.
			}
		}

		return cadena;
	}

	std::vector<double> Linealidad(double* x, double* y, std::vector<int> cadena, std::vector<int> unico, std::vector<int> longitud) {
		//Funcion para calcular la linealidad de las cadenas.
		std::vector<double> linealidad;
		double xi, yi, Rx, Ry, Ixx, Iyy, Ixy, lambda_1, lambda_2, I_max, I_min;
		int posicion = 0;
		double lineal;

		for (int i : unico) {
			Rx = 0;
			Ry = 0;
			Ixx = 0;
			Iyy = 0;
			Ixy = 0;

			//Calculo el centro de gravedad.
			for (int j = 0; j < m_particulas; j++) {
				Rx += x[j] * (cadena[j] == i); //Aqui uso el truco de multiplicar por 0 si la particula j-esima no pertenece a la cadena.
				Ry += y[j] * (cadena[j] == i);
			}

			Rx = Rx / longitud[posicion];
			Ry = Ry / longitud[posicion];

			//Calculo las componentes del tensor de inercia.
			for (int j = 0; j < m_particulas; j++) {
				Ixx += (Ry - y[j]) * (Ry - y[j]) * (cadena[j] == i);
				Iyy += (Rx - x[j]) * (Rx - x[j]) * (cadena[j] == i);
				Ixy -= (Ry - y[j]) * (Rx - x[j]) * (cadena[j] == i);
			}

			//Calculo los autovalores.
			lambda_1 = (Ixx + Iyy + sqrt((Ixx + Iyy) * (Ixx + Iyy) - 4 * (Ixx * Iyy - Ixy * Ixy))) / 2;
			lambda_2 = (Ixx + Iyy - sqrt((Ixx + Iyy) * (Ixx + Iyy) - 4 * (Ixx * Iyy - Ixy * Ixy))) / 2;

			I_max = max(lambda_1, lambda_2);
			I_min = min(lambda_1, lambda_2);

			lineal = (sqrt(I_max) - sqrt(I_min)) / (sqrt(I_max) + sqrt(I_min));

			//Si los autovalores son diferentes e I_max es mayor que 0, calculola linealidad como en el articulo.
			if (!isnan(lineal)) {
				linealidad.push_back(lineal);
			}
			//En caso contrario, es porque la cadena tiene solamente una particula y es circular.
			else {
				linealidad.push_back(0);
			}

			posicion++;
		}

		return linealidad;
	}

public:

	Analisis(double mason, int particulas, int lado) {
		//Constructor de la clase.
		m_mason = mason;
		m_particulas = particulas;
		m_lado = lado;
		m_iteracion = 0;
	}

	void pre_analisis(double* x, double* y) {
		//Funcion que llama a las funciones privadas para analizar y obtener las estadisticas de las cadenas.
		m_iteracion++;
		int* adyacencia = new int[m_particulas * m_particulas];
		adyacencia = Adyacencia(x, y);

		std::vector<int> cadena = BFS(adyacencia);

		std::vector<int> unicos(cadena.size());
		std::vector<int>::iterator it;

		it = std::unique_copy(cadena.begin(), cadena.end(), unicos.begin());
		std::sort(unicos.begin(), it);
		it = std::unique_copy(unicos.begin(), it, unicos.begin());
		unicos.resize(std::distance(unicos.begin(), it));

		std::vector<int> longitud(unicos.size());

		for (size_t i = 0; i < longitud.size(); ++i) {
			longitud[i] = std::count(cadena.begin(), cadena.end(), unicos[i]);
		}

		std::vector<double> linealidad;
		tamannos.push_back(longitud);

		linealidad = Linealidad(x, y, cadena, unicos, longitud);
		linealidades.push_back(linealidad);
		cadenas.push_back(unicos);
	}

	void analisis() {
		//Funcion en la que obtiengo las medias y dispersiones
		int Na;
		double Na_medio = 0;
		double Na_sigma = 0;
		double longitud_media = 0;
		double longitud_sigma = 0;
		double linealidad_media = 0;
		double linealidad_sigma = 0;

		std::vector<double> media(6);
		media[1] = 0;
		media[2] = 0;
		media[5] = 0;

		//Bucle para calcular las medias y las desviaciones estándar.
		for (int i = 0; i < m_iteracion; i++) {
			Na = cadenas[i].size();

			for (int j = 0; j < Na; j++) {
				if (tamannos[i][j] > 1) {
					Na_medio++;
					longitud_media += tamannos[i][j];
					linealidad_media += linealidades[i][j];
				}
			}

			media[0] = Na_medio;
			media[2] = longitud_media / Na_medio;
			media[4] = linealidad_media / Na_medio;

			for (int j = 0; j < Na; j++) {
				if (tamannos[i][j] > 1) {
					Na_medio++;
					longitud_sigma += (tamannos[i][j] - longitud_media) * (tamannos[i][j] - longitud_media);
					linealidad_media += (linealidades[i][j] - linealidad_media) * (linealidades[i][j] - linealidad_media);
				}
			}

			media[1] = 0;
			media[3] = sqrt(longitud_sigma);
			media[5] = sqrt(linealidad_sigma);

			medias.push_back(media);

			Na_medio = 0;
			longitud_media = 0;
			linealidad_media = 0;
		}

	}

	void escribir_analisis(int repeticion) {
		//Funcion para escribir los resultados
		std::ofstream fichero{ "analisis/analisis-" + std::to_string(m_mason) + "-" + std::to_string(repeticion) + ".csv"};

		fichero << "mason,N,Na,Na_sigma,longitud,longitud_sigma,linealidad,linealidad_sigma\n";

		double N = 0;
		double Na_medio = 0;
		double Na_sigma = 0;
		double longitud_media = 0;
		double longitud_sigma = 0;
		double linealidad_media = 0;
		double linealidad_sigma = 0;

		for (int i = 0; i < m_iteracion; i++) {
			N += medias[i][0];
			longitud_media += medias[i][2] * medias[i][0];
			linealidad_media += medias[i][4] * medias[i][0];
		}

		longitud_media = longitud_media / N;
		linealidad_media = linealidad_media / N;
		Na_medio = N / m_iteracion;

		for (int i = 0; i < m_iteracion; i++) {
			Na_sigma += (medias[i][0] - Na_medio) * (medias[i][0] - Na_medio);
			longitud_sigma += (medias[i][0] - 1) * medias[i][3] * medias[i][3] + medias[i][0] * medias[i][2] * medias[i][2];
			linealidad_sigma += (medias[i][0] - 1) * medias[i][5] * medias[i][5] + medias[i][0] * medias[i][4] * medias[i][4];
		}

		Na_sigma = sqrt(Na_sigma);
		longitud_sigma = sqrt((longitud_sigma - N * longitud_media * longitud_media) / (N - m_iteracion));
		linealidad_sigma = sqrt((linealidad_sigma - N * linealidad_media * linealidad_media) / (N - m_iteracion));

		fichero << m_mason << "," << N << "," << Na_medio << "," << Na_sigma << "," << longitud_media << "," << longitud_sigma << "," << linealidad_media << "," << linealidad_sigma << "\n";
	}
};