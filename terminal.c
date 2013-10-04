//*************************************************************************//
//*************************************************************************//
//***************************GAURAV PARIDA*********************************//
//*************************************************************************//
//*****************************201202037***********************************//
//*************************************************************************//
//***************************IIIT HYDERABAD********************************//
//*************************************************************************//
//*******************TERMINAL IMPLEMENTED USING C**************************//
//*************************************************************************//
//*************************************************************************//
#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<malloc.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdlib.h>
//========================================================================//
//========================define the struct===============================//
//========================================================================//
typedef struct node{
	char store[100];
	int live;
	pid_t pid;
}node;
//========================================================================//
//============================function declarations=======================//
//========================================================================//
void parse(char [],char[]);
void redirect_pipe(char []);
void handler(int );
void child_handler(int );
void parse(char [] , char[]);
//========================================================================//
//=============================global variables===========================//
//========================================================================//
pid_t paid;
node lis[100];
char *name;
int k=0;
char host_name[100],extra_path[100];
//========================================================================//
//=======================redirect only====================================//
//========================================================================//
void redirect_only(char asd[])
{
	char *tmp[20];	
	int len=strlen(asd),inpctr=0,outctr=0,inp_flag=0,out_flag=0,i,less_index=-1,great_index=-1;
	char inp_file[100],copy_inp_file[100],out_file[100],copy_out_file[100];

	//==============================check the occureence of the < > in the command================//
	//================================set the index of occurence accordingly======================//
	for(i=0;i<len;i++)
	{
		if(asd[i]=='<' && inp_flag==0)
		{
			inp_flag=1;
			less_index=i;
		}
		else if(asd[i]=='>' && out_flag==0)
		{
			great_index=i;
			out_flag=1;
			inp_flag=0;
		}
		if(inp_flag==1 && asd[i]!='<')
			inp_file[inpctr++]=asd[i];
		if(out_flag==1 && asd[i]!='>')
			out_file[outctr++]=asd[i];
	}
	//====================parsed name of input file and output file need beautification==========//
	inp_file[inpctr]='\0';
	out_file[outctr]='\0';
	parse(inp_file,copy_inp_file);
	parse(out_file,copy_out_file);
	//=======================================done beautifying====================================//
	//================================set the final values of the flags==========================//
	if(inpctr>0)
		inp_flag=-1;
	if(outctr>0)
		out_flag=-1;
	//======================================done setting up of the flags=========================//
	//==================making the input string <null> at the occurence of redirection===========//
	if(inp_flag==-1 && out_flag!=-1)
	{
		asd[less_index]='\0';		
	}
	else if(inp_flag!=-1 && out_flag==-1)
	{
		asd[great_index]='\0';
	}
	else if(inp_flag==-1 && out_flag==-1)
	{
		asd[less_index]='\0';
	}
	//================setting up of the arguments for execvp using the string====================//
	i=0;	
	tmp[0]=strtok(asd," ");
	while((tmp[++i]=strtok(NULL," "))!=NULL);
	//==================================made the arguments ffor execvp===========================//
	//===========================================================================================//
	//=========================if only input redirection is present then=========================//
	if(inp_flag==-1 && out_flag!=-1)
	{
		int len_c=strlen(copy_inp_file);
		copy_inp_file[len_c-1]='\0';
		pid_t pid;
		FILE* in = fopen(copy_inp_file, "r");
		if (in < 0) {
			perror("Can't open the file\n");	
			return;	
		}
		pid = fork();
		if (!pid) {		
			dup2(fileno(in), 0);	
			if (execvp(tmp[0],tmp) < 0) {	
				perror("execution problem\n");
				return;
			}
		}
		else	
			wait(NULL);
	}
	//===========================================================================================//
	//=======================if only the output reirection is present============================//
	else if(inp_flag!=-1 && out_flag==-1)
	{
		int len_c=strlen(copy_out_file);
		copy_out_file[len_c-1]='\0';
		pid_t pid;
		FILE* out = fopen(copy_out_file, "w");
		if (out < 0) {
			perror("can't open the file\n");
			return;			
		}
		pid = fork();
		if (!pid) {		
			dup2(fileno(out), 1);	
			if (execvp(tmp[0],tmp) < 0) {
				perror("execition problem\n");
				return;				
			}
		}
		else			
			wait(NULL);
	}
	//===========================================================================================//
	//=========================if bothe the input and output redirection is present==============//
	else if(inp_flag==-1 && out_flag==-1)
	{
		pid_t pid;
		int len_c=strlen(copy_out_file);
		copy_out_file[len_c-1]='\0';
		len_c=strlen(copy_inp_file);
		copy_inp_file[len_c-1]='\0';
		FILE* out = fopen(copy_out_file, "w");
		FILE* in = fopen(copy_inp_file, "r");
		if (out < 0) {
			perror("cant open the output file\n");
			return;
		}
		if(in <0)
		{
			perror("can't open the input file\n");
			return;	
		}
		pid = fork();
		if (!pid) {				
			dup2(fileno(in),0);	
			dup2(fileno(out),1);
			if (execvp(tmp[0],tmp) < 0) {		
				perror("execl\n");
				return;
			}
		}
		else	
			wait(NULL);
	}
}
//===========================================================================================//
//===========================function handling both piping and redirection===================//
//===========================================================================================//
void redirect_pipe(char as[])
{
	//=========================variables declaration=====================================//
	int status;
	int new_pipe[2];
	int old_pipe[2];
	pid_t pid;
	as[strlen(as)-1]='\0';
	char piped[100][100];
	int pctr=0;
	char *token=strtok(as,"|");
	char copy_str[100];
	//================string evaluation to fill the pipe commands array===================/
	while(token!=NULL)
	{
		parse(token,copy_str);
		copy_str[strlen(copy_str)-1]='\0';
		//printf("%s\n",copy_str);
		strcpy(piped[pctr++],copy_str);
		token=strtok(NULL,"|");
	}
	//========================saving the stdin and stout for later restoring==============//
	int j;
	int stdin_copy, stdout_copy;	
	stdin_copy=dup(STDIN_FILENO);	
	stdout_copy=dup(STDOUT_FILENO);
	//=======================main loop where each piped command array is evaluated=======//
	for(j=0;j<pctr;j++)
	{
		char inp_file[100],out_file[100];
		int less_index=-1,great_index=-1;
		int inp_flag=0,outctr=0;
		int out_flag=0,inpctr=0;
		int i;
		int len = strlen(piped[j]);
		//==========================check the appropriate redirections in the give n=============================================//
		for(i=0;i<len;i++)
		{
			if(piped[j][i]=='<' && inp_flag==0)
			{
				inp_flag=1;
				less_index=i;
			}
			else if(piped[j][i]=='>' && out_flag==0)
			{
				great_index=i;
				out_flag=1;
				inp_flag=0;
			}
			if(inp_flag==1 && piped[j][i]!='<')
				inp_file[inpctr++]=piped[j][i];
			if(out_flag==1 && piped[j][i]!='>')
				out_file[outctr++]=piped[j][i];
		}

		//========================little beautification is required :p===============================//
		char copy_inp_file[100];
		char copy_out_file[100];
		inp_file[inpctr]='\0';
		out_file[outctr]='\0';
		parse(inp_file,copy_inp_file);
		parse(out_file,copy_out_file);
		copy_inp_file[strlen(copy_inp_file)-1]='\0';
		copy_out_file[strlen(copy_out_file)-1]='\0';	
		//==========================set up the final flags for redirection===========================//
		if(inpctr>0)
			inp_flag=-1;
		if(outctr>0)
			out_flag=-1;
		//========================setting up of the input string with null accordingly===============//
		if(inp_flag==-1 && out_flag!=-1)
		{
			piped[j][less_index]='\0';
			FILE* in = fopen(copy_inp_file, "r");
			dup2(fileno(in),0);	
		}
		//==============================only output redirection======================================//
		else if(inp_flag!=-1 && out_flag==-1)
		{
			piped[j][great_index]='\0';
			FILE* out = fopen(copy_out_file, "w");
			dup2(fileno(out),1);

		}
		//================================both input and output redirection is present===============//
		else if(inp_flag==-1 && out_flag==-1)
		{
			piped[j][less_index]='\0';
			FILE* out = fopen(copy_out_file, "w");
			FILE* in = fopen(copy_inp_file, "r");
			dup2(fileno(in),0);
			dup2(fileno(out),1);
		}
		//===============================set up the pipes!!==========================================//
		if(j < pctr-1)
		{	
			pipe(new_pipe);
		}
		//==================================forking the things=======================================//
		pid = fork();
		//=======================not applicable to the starting pipe and all the others==============//
		if(j>0 && j<=pctr-1)
		{
			dup2(old_pipe[0], 0);
			close(old_pipe[1]);
			close(old_pipe[0]);
		}
		//===================================working out==============================================//
		if(pid == 0) 
		{
			if(j>=0 && j<pctr-1)
			{
				dup2(new_pipe[1], 1);
				close(new_pipe[0]);
				close(new_pipe[1]);
			}
			//==================================tokenising the final beautiful string==============//
			char *tmp[20];
			int alp=0;
			tmp[0]=strtok(piped[j]," ");
			while((tmp[++alp]=strtok(NULL," "))!=NULL);
			//=============================executing it accorddingly===============================//
			if (execvp(tmp[0],tmp) < 0) 
			{
				perror("execution problem\n");
				exit(1);
			}
		} 
		//=================================father process==============================================//
		else 
		{		
			waitpid(pid, &status, 0);
			if(j < pctr-1) 
			{
				old_pipe[0] = new_pipe[0];
				old_pipe[1] = new_pipe[1];
			}
		}
	}
	//=======================restroing the previous settings and clearing the pipes=======================//
	close(old_pipe[0]);
	close(new_pipe[0]);
	close(old_pipe[1]);
	close(new_pipe[1]);
	dup2(stdin_copy, 0);
	dup2(stdout_copy, 1);
	close(stdin_copy);
	close(stdout_copy);
	//=======================================E_O_Function==========================================================//
	return;


}		
//-----------------------------------------------------------------------------
void handler(int sig)
{
	if(sig==20)
	{
		kill(paid,20);
		printf("\n");
	}
	else if(sig==2 || sig==3){
		fflush(stdout);
		printf("\n");
		printf("<%s@%s:%s>",name,host_name,extra_path);
		fflush(stdout);
	}
	return;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void parse(char a[],char test[])
{
	int i,j;
	char sec_str[100][100];
	int sec_ptr=0;
	int len=strlen(a);
	j=0;
	for(i=0;i<len;i++)
	{
		j=0;
		int check=0;
		while(a[i]!=' ' && a[i]!='\t' && a[i]!='\n')
		{
			sec_str[sec_ptr][j++]=a[i];
			i++;
			check=1;
		}
		//if(a[i]==' ' || a[i]=='\n' || a[i]=='\t' || a[i]=='\0')
		//	check=1;
		if(check==1)
		{
			sec_str[sec_ptr][j]='\0';	
			sec_ptr++;
		}

	}

	sec_str[sec_ptr][j]='\0';	
	//sec_ptr--;
	/*for(i=0;i<sec_ptr;i++)
	  {
	//printf("%s\n",sec_str[i]);
	}*/
	//printf("test is perdect\n");
	int test_ptr=0;
	for(i=0;i<sec_ptr;i++)
	{
		for(j=0;j<strlen(sec_str[i]);j++)
		{
			test[test_ptr++]=sec_str[i][j];		
		}
		test[test_ptr++]=' ';	
	}
	//printf("test is perdect\n");
	test[test_ptr]='\0';
	//printf("test is perdect\n");
}
//----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void child_handler(int signum)
{	
	int p;
	int i;
	char *st;
	p = waitpid(-1,&st , WNOHANG);
	if(p>0){
		for(i=0;i<k;i++)
		{
			if(lis[i].pid==p)
			{
				printf("\nExited Normally %s %d\n",lis[i].store,lis[i].pid);
				printf("<%s@%s:%s>",name,host_name,extra_path);
				lis[i].live=1;
				break;
			}
		}
		fflush(stdout);
	}
	else{
		signal(SIGCHLD, SIG_IGN);
	}

}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int main()
{
	name = getenv("USER");
	char *tmp[20];
	gethostname(host_name,100);
	char work_dir[100];
	getcwd(work_dir,100);
	char pathdir[100];
	getcwd(pathdir,100);
	//ptr of the path which is assumed to be the ~	
	int fixed_len=strlen(work_dir);
	int len;
	char pathdir2[100];
	strcpy(pathdir2,pathdir);
	while(1)	
	{

		signal(SIGINT,SIG_IGN); 
		signal(SIGINT,handler); 
		signal(SIGQUIT,SIG_IGN); 
		signal(SIGQUIT,handler); // signal(SIGTSTP,SIG_IGN); 
		signal(SIGTSTP,handler); 
		signal(SIGCHLD,SIG_IGN); 
		signal(SIGCHLD,child_handler);
		len=strlen(work_dir);
		char string[100];
		if(strstr(work_dir,pathdir)!=NULL)
		{
			int ctr=1,i;
			for(i=fixed_len;i<len;i++)
			{
				extra_path[ctr++]=work_dir[i];
			}
			extra_path[0]='~';
			extra_path[ctr]='\0';
		}
		else{
			char a[100];
			getcwd(a,100);
			strcpy(extra_path,a);
		}
		printf("<%s@%s:%s>",name,host_name,extra_path);
		//-------------------------input stuff---------------------
		int i=0;
		int ret;
		//---------------------------------------------------------
		char copy_string[100];		
		fgets(string, sizeof(string), stdin);
		//-------------------check back or forground-----------------
		//int flag=1;
		//-----------------------------------------------------------
		parse(string,copy_string);
		//===================Check if redirection present========================================//
		int len2=strlen(copy_string);
		int m;
		int redirect_ctr=0;
		for(m=0;m<len2;m++)
		{
			if(copy_string[m]=='<' || copy_string[m]=='>')
				redirect_ctr++;	
		}	
		//=======================Check if piping is present===============================//
		int pipe_ctr=0;		
		len2=strlen(copy_string);
		for(m=0;m<len2;m++)
		{
			if(copy_string[m]=='|')
				pipe_ctr++;
		}
		//=========================Resolving accordingly==================================//
		if(redirect_ctr>0 && pipe_ctr==0)
		{
			//printf("inly redirection\n");
			redirect_only(copy_string);
			continue;
		}
		else if(redirect_ctr>=0 && pipe_ctr>0 )
		{
			//printf("pipe\n");
			redirect_pipe(copy_string);
			continue;
		}
		//=============================otherwise normal operation========================//

		int flag=1;
		m=0;
		len2=strlen(copy_string);
		for(m=0;m<len2;m++)
		{
			if(copy_string[m]=='&')
			{
				copy_string[m]='\0';
				flag=0;
			}
		}
		strcpy(string,copy_string);
		//======================removing the command and the list of arguments============//
		tmp[0]=strtok(string," ");
		while((tmp[++i]=strtok(NULL," "))!=NULL);
		if(strlen(string)==0)//simple entr avoid
		{
			continue;
		}	
		//================================================================================//
		//=====================================exit=======================================//
		if(strcmp(tmp[0],"quit")==0){
			exit(0);
		}
		//================================================================================//
		//=================================change directory===============================//
		if(strcmp(tmp[0],"cd")==0){
			if(i==1)//no argument is given
			{
				paid = fork();
				if (paid > 0){
					ret = chdir(pathdir);
					getcwd(work_dir,100);
					int status;
					wait(&status);
				}
				else if(paid == 0)
					_exit(0);
				continue;
			}
			else //argument of the path is given
			{
				paid = fork();
				if(paid > 0){
					ret = chdir(tmp[1]);
					getcwd(work_dir,100);
					int status;
					wait(&status);
				}
				if(paid == 0){
					_exit(0);
				}
				if(paid < 0)
					printf("wrong loaction\n");
				continue;
			}
		}
		//================================================================================//
		//---------------------------------------kjob--------------------------------------------------
		else if(strcmp(tmp[0],"kjob")==0)
		{
			if((tmp[0]!=NULL) & (tmp[1]!=NULL))
			{
				int max_oc=atoi(tmp[1]);
				int ctr5=0;
				int n;
				//int count;
				for(n=0;n<k;n++)
				{
					if(lis[n].live==0)
					{
						ctr5++;
					}
					if(ctr5==max_oc)
					{
						max_oc=n;
						break;
					}

				}
				if(ctr5==atoi(tmp[1]))
				{
					kill(lis[max_oc].pid,atoi(tmp[2]));
				}
				else{
					printf("insufficient jobs\n");
				}
			}
			else
			{
				printf("Improper arguments\n");
			}
		}
		//-------------------------------------------------------------------------------------------
		//---------------------------------------oveerkill--------------------------------------------------
		else if(strcmp(tmp[0],"overkill")==0)
		{
			int i;
			for(i=0;i<k;i++)
			{
				if(lis[i].live==0)
				{
					printf("killed the process %s %d\n",lis[i].store,lis[i].pid);
					kill(lis[i].pid,9);
				}
			}
		}
		//-------------------------------------------------------------------------------------------
		//---------------------------------------fg--------------------------------------------------
		else if(strcmp(tmp[0],"fg")==0)
		{
			//printf("fg \n");
			if(tmp[1]!=NULL && tmp[2]==NULL)
			{
				int max_of=atoi(tmp[1]);
				int count=0;
				int fl=0;
				for(i=0;i<k;i++)
				{
					if(lis[i].live==0)
					{
						count++;
					}
					if(count==max_of)
					{
						max_of=i;
						fl=1;
						break;
					}
				}
				if(fl==1)
				{
					int st;
					while(1)
					{

						int par=waitpid(lis[max_of].pid,&st,WNOHANG);
						if(par>0)
						{
							lis[max_of].live=1;
							printf("\nExited Normally %s %d\n",lis[max_of].store,lis[max_of].pid);
							break;
						}
					}
				}
				else
				{
					printf("Lack of argument\n");
				}
			}
			else
			{
				printf("Invalid arguments\n");
			}
		}
		//-------------------------------------------------------------------------
		//-------------------------------------------------------------------------
		//-------------------------------------------------------------------------
		else if(strcmp(tmp[0],"jobs")==0)
		{
			int ctr4=1;
			for(i=0;i<k;i++)
			{
				if(lis[i].live==0)
				{
					printf("[%d] %s [%d]\n",ctr4++,lis[i].store,lis[i].pid);
				}
			}
		}
		//-------------------------------------------------------------------------------------------------
		//.....................................pinfo.......................................................
		else if(strcmp(tmp[0],"pinfo")==0)
		{
			if(tmp[2]!=NULL)
			{
				printf("put proper arguments\n");
				continue;
			}
			int p;
			if(tmp[1])
			{
				p = atoi(tmp[1]);
			}
			else
			{
				printf("PID of the terminal is %d\n",getpid());
				p=getpid();
			}
			int ctr=0;
			char tmpo[100],tmp2[100];
			while(p>0)
			{
				tmpo[ctr++]=p%10 + '0';
				p/=10;
			}
			int ctr3=0;
			for(i=ctr-1;i>=0;i--)
			{
				tmp2[ctr3++]=tmpo[i];
			}
			tmp2[ctr3]='\0';
			char fp[100]={0};
			strcat(fp,"/proc/");
			strcat(fp,tmp2);
			strcat(fp,"/status");
			FILE *fo;
			fo=fopen(fp,"r");
			if(fo>0)
			{
				char char_read;
				int ctr2=0;
				while(!feof(fo))
				{
					fscanf(fo,"%c",&char_read);
					//read(fo,&char_read,1);
					if(char_read=='\n')
					{
						ctr2++;
					}
					if(ctr2<=2 || ctr2==4 || ctr2==11)
					{
						write(2,&char_read,1);
					}
				}
				printf("\n");
				fclose(fo);
			}
			else
			{
				printf("No such process exist\n");
			}

			char ans_exe[100],ans_exe1[100]={0};
			strcat(ans_exe1,"/proc/");
			strcat(ans_exe1,tmp2);
			strcat(ans_exe1,"/exe");
			strcat(ans_exe1,"\0");
			memset(ans_exe,0,sizeof(ans_exe));
			if(readlink(ans_exe1,ans_exe,sizeof(ans_exe)-1))
			{
			}
			printf("Executable Path : %s\n",ans_exe);
			continue;
		}
		//-------------------------------------------------------------
		//.............................................................
		//-------------------------------------------------------------
		else//for any command
		{
			paid = fork();
			if(paid==0)
			{
				execvp(tmp[0],tmp);
				_exit(0);
			}
			else if(paid > 0)
			{
				int *st;
				if (flag==1)
				{
					int alp=waitpid(paid,&st,WUNTRACED);
					if(alp>0){
						strcpy(lis[k].store,copy_string);
						lis[k].live=1;
						lis[k].pid=paid;
						k++;
					}
				}
				else // for handling background process
				{
					lis[k].pid=paid;
					strcpy(lis[k].store,copy_string);
					lis[k].live=0;
					printf("command %s pid %d\n",tmp[0],lis[k].pid);
					k++;
				}
			}
			else if(paid < 0)
			{
				printf("Unknown error\n");
			}
			continue;

		}		
	}
}
