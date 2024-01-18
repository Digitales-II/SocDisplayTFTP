#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <system.h>
#include <irq.h>

//#include "ethernet.h"

#include <generated/csr.h>
#include <generated/soc.h>
#include <generated/mem.h>
#include <generated/git.h>

#include <libbase/console.h>
#include <libbase/crc.h>
#include <libbase/memtest.h>

#include <libbase/spiflash.h>
#include <libbase/uart.h>
#include <libbase/i2c.h>

#include <liblitedram/sdram.h>
#include <liblitedram/utils.h>

#include <libliteeth/udp.h>
#include <libliteeth/mdio.h>

#include <liblitespi/spiflash.h>

#include <liblitesdcard/sdcard.h>
#include <liblitesata/sata.h>



void clean_memories()
{
	for (int dir = 0; dir < 3200; dir++)
	{
		pantalla_addrWrite_write(dir);
		pantalla_dataLine_write(0);

	}	
}
//--------------Funciones para recibir en la memoria archivos de un servidor tftp ------------------
static unsigned int remote_ip[4] = {192, 168, 1, 100};
static unsigned int local_ip[4] = {192, 168, 1, 50};

static uint8_t parse_ip(const char * ip_address, unsigned int * ip_to_change)
{
	uint8_t n = 0;
	uint8_t k = 0;
	uint8_t i;
	uint8_t size = strlen(ip_address);
	unsigned int ip_to_set[4];
	char buf[3];

	if (size < 7 || size > 15) {
		printf("Error: Invalid IP address length.");
		return -1;
	}

	/* Extract numbers from input, check for potential errors */
	for (i = 0; i < size; i++) {
		if ((ip_address[i] == '.' && k != 0) || (ip_address[i] == '\n' && i == size - 1)) {
			ip_to_set[n] = atoi(buf);
			n++;
			k = 0;
			memset(buf, '\0', sizeof(buf));
		} else if (ip_address[i] >= '0' && ip_address[i] <= '9' && k < 3) {
			buf[k] = ip_address[i];
			k++;
		} else {
			printf("Error: Invalid IP address format. Correct format is \"X.X.X.X\".");
			return -1;
		}
	}
	ip_to_set[n] = atoi(buf);

	/* Check if a correct number of numbers was extracted from the input*/
	if (n != 3) {
		printf("Error: Invalid IP address format. Correct format is \"X.X.X.X\".");
		return -1;
	}

	/* Set the extracted IP address as local or remote ip */
	for (i = 0; i <= n; i++) {
		ip_to_change[i] = ip_to_set[i];
	}
	return 0;
}


void set_local_ip(const char * ip_address)
{
	if (parse_ip(ip_address, local_ip) == 0) {
		udp_set_ip(IPTOINT(local_ip[0], local_ip[1], local_ip[2], local_ip[3]));
		printf("Local IP: %d.%d.%d.%d", local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
	}
}

void set_remote_ip(const char * ip_address)
{
	if (parse_ip(ip_address, remote_ip) == 0) {
		printf("Remote IP: %d.%d.%d.%d", remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3]);
	}
}

int copy_file_from_tftp_to_ram(unsigned int ip, unsigned short server_port, const char *filename, char *buffer)
{
	int size;
	printf("Copying %s to %p... ", filename, buffer);
	size = tftp_get(ip, server_port, filename, buffer);
	if(size > 0){
		printf("(%d bytes)", size);
		printf("\n The data was sent correctly \n");}
	else{
	printf("\n failed Connection \n");}
	return size;
}
//--------------------------------------------------------------------------------------------------

//---------------------Funcion para actualizar las lineas de informacion de la pantalla-------------

#define BITS_PER_BYTE 8
#define COUNTER_LIMIT 5
long long int updateLine(int Line) {
    int counter = 0;
    long long int data = 0;
    unsigned char *memory_address = (unsigned char *)0x40200000;

    for (counter = 0; counter <= COUNTER_LIMIT; ++counter) {
        unsigned char byte = memory_address[counter + 6 * Line];
        data = (data << BITS_PER_BYTE) | byte;
    }

    //printf("%llx\n", data);

    return data;
}
//--------------------------------------------------------------------------------------------------

//--------------------------------------Delays------------------------------------------------------
void my_busy_wait(unsigned int ms)
{
	timer0_en_write(0);
	timer0_reload_write(0);
	timer0_load_write(CONFIG_CLOCK_FREQUENCY/1000*ms);
	timer0_en_write(1);
	timer0_update_value_write(1);
	while(timer0_value_read()) timer0_update_value_write(1);
}

//--------------------------------------------------------------------------------------------------
__attribute__((__used__)) int main(int i, char **c)
{
	
	static unsigned int connection_ip[4] = {192, 168, 1, 100};
	unsigned int ip;
	
	int addr=0;
#define TFTP_SERVER_PORT 69
	

#ifdef CONFIG_CPU_HAS_INTERRUPT
	irq_setmask(0);
	irq_setie(1);
#endif
	uart_init();

#ifdef CONFIG_HAS_I2C
	i2c_send_init_cmds();
#endif

	printf("\n");
	printf("\e[1m:::::::'######:::::'###::::'####:'##::: ##:\e[0m\n");
	printf("\e[1m::::::'##... ##:::'## ##:::. ##:: ###:: ##:\e[0m\n");
	printf("\e[1m:::::: ##:::..:::'##:. ##::: ##:: ####: ##:\e[0m\n");
	printf("\e[1m:::::: ##:::..:::'##:. ##::: ##:: ####: ##:\e[0m\n");
	printf("\e[1m:::::: ##:::..:::'##:. ##::: ##:: ####: ##:\e[0m\n");
	printf("\e[1m:::::: ##:::::::'##:::. ##:: ##:: ## ## ##:\e[0m\n");
	printf("\e[1m:::::: ##::::::: #########:: ##:: ##. ####:\e[0m\n");
	printf("\e[1m:::::: ##::: ##: ##.... ##:: ##:: ##:. ###:\e[0m\n");
	printf("\e[1m::::::. ######:: ##:::: ##:'####: ##::. ##:\e[0m\n");
	printf("\e[1m:::::::......:::..:::::..::....::..::::..::\e[0m\n");
	printf("\n");

	printf("\e[1m Modified and updated by: \e[0m\n");
	printf("\e[1m Esteban Peña : \e[0m\n");
	printf("\e[1m Jhoan Clavijo: \e[0m\n");

	printf("\n");
	printf("--============= \e[1m Controller\e[0m ================--\n");
	printf("--===== \e[1m Project Display-SoC-LM32 \e[0m =========--\n");
	printf("\n");
	
	//Registros de estatus y control(SCR), periferio pantalla 
	pantalla_enable_write(1); 
	pantalla_wr_write(1);
	pantalla_RamTime_write(2*41250000);
	//Asignar IPs
	set_local_ip("192.168.1.50");
	set_remote_ip("192.168.1.100");
	
	
	//Comunicación y recepcion de informacion en la memoria por TFTP
	int size;
	ip = IPTOINT(connection_ip[0], connection_ip[1], connection_ip[2], connection_ip[3]);
	size=copy_file_from_tftp_to_ram(ip, TFTP_SERVER_PORT, "SAlida.bin", (void *)0x40200000);
	printf("(%d) \n", size);

	

	clean_memories();//Limpiar pantalla
	
	while(1) {
		
		while(1){
			if (addr>=3200){
				break;
			}
			pantalla_addrWrite_write(addr);//direccion
			pantalla_dataLine_write(updateLine(addr));//Actualizar fila de informacion
			addr++;
		}
		
		//printf("FIN");
		

	}
	
	return 0;
}
