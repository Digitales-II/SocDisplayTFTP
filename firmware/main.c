// This file is Copyright (c) 2013-2014 Sebastien Bourdeauducq <sb@m-labs.hk>
// This file is Copyright (c) 2014-2019 Florent Kermarrec <florent@enjoy-digital.fr>
// This file is Copyright (c) 2015 Yann Sionneau <ys@m-labs.hk>
// This file is Copyright (c) 2015 whitequark <whitequark@whitequark.org>
// This file is Copyright (c) 2019 Ambroz Bizjak <ambrop7@gmail.com>
// This file is Copyright (c) 2019 Caleb Jamison <cbjamo@gmail.com>
// This file is Copyright (c) 2018 Dolu1990 <charles.papon.90@gmail.com>
// This file is Copyright (c) 2018 Felix Held <felix-github@felixheld.de>
// This file is Copyright (c) 2019 Gabriel L. Somlo <gsomlo@gmail.com>
// This file is Copyright (c) 2018 Jean-François Nguyen <jf@lambdaconcept.fr>
// This file is Copyright (c) 2018 Sergiusz Bazanski <q3k@q3k.org>
// This file is Copyright (c) 2016 Tim 'mithro' Ansell <mithro@mithis.com>
// This file is Copyright (c) 2020 Franck Jullien <franck.jullien@gmail.com>
// This file is Copyright (c) 2020 Antmicro <www.antmicro.com>

// License: BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <system.h>
#include <irq.h>

#include "boot.h"
#include "readline.h"
#include "helpers.h"
#include "command.h"

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


//---------------------Funcion para actualizar las lineas de informacion de la pantalla-------------

long long int updateLine(int Line){
	int a=0;
	char data[54];
	while(1){
		if(a<=53){
			volatile unsigned char da=((volatile unsigned char *) 0x40200000)[a+55*Line];
			data[a]=da;
			a++;
		}
		else{
			long long int decimalNumber = strtoll(data,NULL, 2);
			return decimalNumber;}
	}
}
//--------------------------------------------------------------------------------------------------

//Actualización de las lineas de informacion en binario
/*
long long int updateLine(int Line){
	int a=0;
	long long int data[54];
	while(1){
		if(a<=53){
			volatile unsigned int da=((volatile unsigned char *) 0x40200000)[a+55*Line];
			data[a]=da;
			a++;
		}
		else{
			long long int decimalNumber = data;
			return decimalNumber;}
	}
}*/

void my_busy_wait(unsigned int ms)
{
	timer0_en_write(0);
	timer0_reload_write(0);
	timer0_load_write(CONFIG_CLOCK_FREQUENCY/1000*ms);
	timer0_en_write(1);
	timer0_update_value_write(1);
	while(timer0_value_read()) timer0_update_value_write(1);
}


__attribute__((__used__)) int main(int i, char **c)
{
	char buffer[CMD_LINE_BUFFER_SIZE];
	char *params[MAX_PARAM];
	char *command;
	struct command_struct *cmd;
	int nb_params;
	int sdr_ok;
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
	printf("--============= \e[1mController\e[0m ================--\n");
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
	size=copy_file_from_tftp_to_ram(ip, TFTP_SERVER_PORT, "data.txt", (void *)0x40200000);
	printf("(%d) \n", size);

	init_dispatcher();
	printf("\n%s", PROMPT);

	clean_memories();//Limpiar pantalla
	
	while(1) {
		
		if(addr<3200){
			pantalla_addrWrite_write(addr);//direccion
			pantalla_dataLine_write(updateLine(addr));//Actualizar fila de informacion
			addr++;
		}
	
	}
	
	return 0;
}
