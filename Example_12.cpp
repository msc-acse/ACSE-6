#include <mpi.h>
#include <iostream>
#include <locale>

using namespace std;

int id, p;

MPI_Datatype MPI_my_class;

class my_class
{
public:
	int I1, I2;
	int var_not_to_send;
	double D1;
	char S1[50];

	static void buildMPIType();
};

void my_class::buildMPIType()
{
	int block_lengths[4];
	MPI_Aint displacements[4];
	MPI_Aint addresses[4], add_start;
	MPI_Datatype typelist[4];

	my_class temp;

	typelist[0] = MPI_INT;
	block_lengths[0] = 1;
	MPI_Get_address(&temp.I1, &addresses[0]);

	typelist[1] = MPI_INT;
	block_lengths[1] = 1;
	MPI_Get_address(&temp.I2, &addresses[1]);

	typelist[2] = MPI_DOUBLE;
	block_lengths[2] = 1;
	MPI_Get_address(&temp.D1, &addresses[2]);

	typelist[3] = MPI_CHAR;
	block_lengths[3] = 50;
	MPI_Get_address(&temp.S1, &addresses[3]);

	MPI_Get_address(&temp, &add_start);
	for (int i = 0; i < 4; i++) displacements[i] = addresses[i] - add_start;

	MPI_Type_create_struct(4, block_lengths, displacements, typelist, &MPI_my_class);
	MPI_Type_commit(&MPI_my_class);
}

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	my_class::buildMPIType();

	my_class *data=new my_class[10];

	if (id == 0)
	{
		for (int i = 0; i < 10; i++)
		{
			data[i].I1 = 6;
			data[i].I2 = 3;
			data[i].D1 = 10.0;
			data[i].var_not_to_send = 25;
			strncpy(data[i].S1, "My test string", 50);
		}
	}

	MPI_Bcast(data, 10, MPI_my_class, 0, MPI_COMM_WORLD);

	cout << "On process " << id << endl;
	for (int i = 0; i < 10; i++)
		cout<<"\t"<< i << ": I1=" << data[i].I1 << " I2=" << data[i].I2 << " D1= " << data[i].D1 << " S1=" << data[i].S1 << ". The unsent variable is " << data[i].var_not_to_send << endl;

	MPI_Type_free(&MPI_my_class);
	MPI_Finalize();
}