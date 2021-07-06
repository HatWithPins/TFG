//Clase de la caja que contiene las partículas.
class Caja {
private:
	int m_particulas; //Número de partículas.
	int m_lado; //Lado de la caja, un múltiplo del diámetro de las partículas.
	std::vector<std::vector<double>> posiciones; //Vector de dos dimensiones que contiene las posiciones de las partículas.

	//Función para calcular las posiciones iniciales de las partículas.
	void posicionInicial() {
		//Genero primero un vector de una dimensión con números del 0 hasta el cuadrado del lado menos 1.
		std::vector<int> posiciones_iniciales{};
		std::vector<double> x{};
		std::vector<double> y{};

		for (int i = 0; i < m_lado * m_lado; i++) {
			posiciones_iniciales.push_back(i);
		}

		//Reordeno aleatoriamente los elementos del vector.
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(posiciones_iniciales.begin(), posiciones_iniciales.end(), std::default_random_engine(seed));

		//Por cada posición inicial, calculo sus coordenadas y las añado al vector bidimensional.
		for (int i = 0; i < m_particulas; i++) {
			x.push_back(calcularXInicial(posiciones_iniciales[i]));
			y.push_back(calcularYInicial(posiciones_iniciales[i]));
		}
		posiciones.push_back(x);
		posiciones.push_back(y);
	}

	/*El algoritmo de las posiciones iniciales divide la caja en trozos iguales. En total, m_lado^2 trozos, numerados desde el 0.
	Para cada número, calculo las coordenadas equivalentes.
	Para x, la operación es posicion módulo m_lado + 1/2.
	Para y, la operación es posicion/m_lado + 1/2, es una división entera.
	Esto significa que el centro de la partícula estará en el centro del trozo correspondiente y que diferentes partículas tendrán diferentes posiciones iniciales.*/
	double calcularXInicial(int posicion) {
		return posicion % m_lado + 0.5;
	}

	double calcularYInicial(int posicion) {
		int division = posicion / m_lado;
		return division + 0.5;
	}

	//Funciones para obtener la posición de una partícula dada. En principio, son para uso interno.
	double getX(int particula) {
		return posiciones[0][particula];
	}
	double getY(int particula) {
		return posiciones[1][particula];
	}

public:

	//Función para crear la caja en su estado inicial.
	void crearCaja(int particulas, int lado) {
		m_particulas = particulas;
		m_lado = lado;

		posicionInicial();
	}

	//Función para escribir las posiciones en un fichero csv.
	void escribirPosiciones(int iteracion, double mason, int repeticion) {
		std::ofstream fichero{ "posiciones/posiciones-" + std::to_string(mason) + "-" + std::to_string(repeticion) + "-" + std::to_string(iteracion) + ".csv"};

		fichero << "x,y\n";

		for (int i = 0; i < m_particulas; i++) {
			fichero << getX(i) << "," << getY(i) << "\n";
		}
	}

	std::vector<double> returnX() {
		return posiciones[0];
	}

	std::vector<double> returnY() {
		return posiciones[1];
	}

	void setX(double* x) {
		for (int i = 0; i < m_particulas; i++) {
			posiciones[0][i] = x[i];
		}
	}

	void setY(double* y) {
		for (int i = 0; i < m_particulas; i++) {
			posiciones[1][i] = y[i];
		}
	}
};