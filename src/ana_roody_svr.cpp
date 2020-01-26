#include "ana_roody_svr.h"
#define POINTER_T unsigned long

ana_roody_svr::ana_roody_svr()
{
	thread_id = 3;
	real_stat = DAQ_STOP;
	tsock = NULL;
	tlsock = NULL;
	init_fun.push_back(&ana_roody_svr_init);
}

ana_roody_svr::~ana_roody_svr()
{
	if (tsock)
		delete tsock;
	if (tlsock)
		delete tlsock;
}

int ana_roody_svr::ana_roody_svr_init(my_thread* This, initzer* the_initzer)
{
	ana_roody_svr* ptr = reinterpret_cast<ana_roody_svr*>(This);

	ptr->port = the_initzer->get_ana_roody_svr_port();
	ptr->tlsock = new TServerSocket(ptr->port, true);
	ptr->tlsock->SetOption(kNoBlock, 1);
	ptr->acq_stat = DAQ_RUN;
	
	return 0;
}

int ana_roody_svr::handle_msg(uint32_t* msg_body)
{
	/* The message type of the current thread are defined as following 
	 * 1 --> run status transition
	 * 2 --> Need to establish the connection.
	 * 3 --> To be defined...
	 * */
	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	switch (msg_type) {
	case 1:
		/* run status transition
		 * */
		return switch_run(msg_body[1]);
	case 2:
		/* accept an incoming connection. */
		return conn_roody();
	default:
		return -E_MSG_TYPE;
	}

}

int ana_roody_svr::conn_roody()
{
	tsock = tlsock->Accept();
	if (((long)tsock) == -1) {
		/* No incoming connection request. */
		tsock = NULL;
		return send_msg(thread_id, 2, NULL, 0);
	}
	else if (tsock == NULL) {
		return -E_SYSCALL;
	}
	tsock->SetOption(kNoBlock, 1);

	return 0;
}
int ana_roody_svr::start()
{
	int ret;
	real_stat = DAQ_RUN;
	
	/* we need to establish the connectiong between the client and current
	 * thread. */
	if (!tsock) {
		/* The connection is not established yet, however, we don't
		 * call 'accept' here since it block, we just send a message to
		 * mark the necessity to 'accept' an incoming connection
		 * request */
		ret = send_msg(thread_id, 2, NULL, 0);
		RET_IF_NONZERO(ret);
	}

	/* proporgate the 'start' message to next thread. */
	return send_msg(2, 1, &real_stat, 4);
}

int ana_roody_svr::stop()
{
	real_stat = DAQ_STOP;
	return send_msg(4, 1, &real_stat, 4);
}

int ana_roody_svr::quit()
{
	acq_stat = DAQ_EXIT;
	real_stat = DAQ_EXIT;
	return 0;
}

static TFolder *ReadFolderPointer(TSocket * fSocket)
{
	//read pointer to current folder
	TMessage *m = 0;
	fSocket->Recv(m);
	POINTER_T p;
	*m >> p;
	return (TFolder *) p;
}
int ana_roody_svr::main_proc()
{
	/* In this function, the current thread receives and send messages to
	 * roody. (codes copied from MIDAS)*/
	char request[256];
	int ret;

	if (!tsock) {
		/* connection not established yet! */
		usleep(120);
		return 0;
	}

	/* close connection if client has disconnected */
	ret = tsock->Recv(request, sizeof(request));
	if (ret <= 0) {
		if (ret == -4) {
			/* Nothing to be received */
			usleep(1000);
			return 0;
		}
	// printf("Closed connection to %s\n", 
	//         sock->GetInetAddress().GetHostName());
		/* otherwise the connection must have been closed */
		tsock->Close();
		delete tsock;
		tsock = NULL;
		return send_msg(thread_id, 2, NULL, 0);

	} 

	/* Now we've received a message from roody. */
	TMessage *message = new TMessage(kMESS_OBJECT);
	if (strcmp(request, "GetListOfFolders") == 0) {
		TFolder *folder = ReadFolderPointer(tsock);
		if (folder == NULL) {
			/* empty folder */
			message->Reset(kMESS_OBJECT);
			message->WriteObject(NULL);
			tsock->Send(*message);
			delete message;
			return 0;
		}
		
		/* get folder names */
		TObject *obj;
		TObjArray *names = new TObjArray(100);

		TCollection *folders = folder->GetListOfFolders();
		TIterator *iterFolders = folders->MakeIterator();
		while ((obj = iterFolders->Next()) != NULL)
			names->Add(new TObjString(obj->GetName()));

		/* write folder names */
		message->Reset(kMESS_OBJECT);
		message->WriteObject(names);
		tsock->Send(*message);

		/* clean things up */
		for (int i = 0; i < names->GetLast() + 1; i++)
			delete(TObjString *) names->At(i);
		delete names;
		delete message;
	} 
	else if (strncmp(request, "FindObject", 10) == 0) {
		TFolder *folder = ReadFolderPointer(tsock);
	
		/* get object */
		TObject *obj;
		if (strncmp(request + 10, "Any", 3) == 0)
			obj = folder->FindObjectAny(request + 14);
		else
			obj = folder->FindObject(request + 11);

		/* write object */
		if (!obj)
			tsock->Send("Error");
		else {
			message->Reset(kMESS_OBJECT);
			message->WriteObject(obj);
			tsock->Send(*message);
		}

		/* clean up */
		delete message;

	} else if (strncmp(request, "FindFullPathName", 16) == 0) {
		TFolder *folder = ReadFolderPointer(tsock);
            
		/* find path */
		const char *path = folder->FindFullPathName(request + 17);

		/* write path */
		if (!path) {
			tsock->Send("Error");
		} 
		else {
			TObjString *obj = new TObjString(path);
			message->Reset(kMESS_OBJECT);
			message->WriteObject(obj);
			tsock->Send(*message);
			delete obj;
		}

		/* clean up */
		delete message;

	} else if (strncmp(request, "Occurence", 9) == 0) {
		TFolder *folder = ReadFolderPointer(tsock);

		/* read object */
		TMessage *m = 0;
		tsock->Recv(m);
		TObject *obj = ((TObject *) m->ReadObject(m->GetClass()));

		/* get occurence */
		Int_t retValue = folder->Occurence(obj);

		/* write occurence */
		message->Reset(kMESS_OBJECT);
		*message << retValue;
		tsock->Send(*message);

		/* clean up */
		delete message;

         } else if (strncmp(request, "GetPointer", 10) == 0) {
		/* find object */
		TObject *obj = gROOT->FindObjectAny(request + 11);

		//write pointer
		message->Reset(kMESS_ANY);
		POINTER_T p = (POINTER_T) obj;
		*message << p;
		tsock->Send(*message);

		/* clean up */
		delete message;

	} else if (strncmp(request, "Command", 7) == 0) {
		char objName[100], method[100];
		tsock->Recv(objName, sizeof(objName));
		tsock->Recv(method, sizeof(method));
		TObject *object = gROOT->FindObjectAny(objName);
		if (object && object->InheritsFrom(TH1::Class())
			&& strcmp(method, "Reset") == 0)
			static_cast < TH1 * >(object)->Reset();

	} else if (strncmp(request, "SetCut", 6) == 0) {
		/* read new settings for a cut */
		char name[256];
		tsock->Recv(name, sizeof(name));
		/* The cut is set to NULL so that this feature is disabled... */
		TCutG *cut = NULL;

		TMessage *m = 0;
		tsock->Recv(m);
		TCutG *newc = ((TCutG *) m->ReadObject(m->GetClass()));

		if (cut) {
		newc->TAttMarker::Copy(*cut);
		newc->TAttFill::Copy(*cut);
		newc->TAttLine::Copy(*cut);
		newc->TNamed::Copy(*cut);
		cut->Set(newc->GetN());
		for (int i = 0; i < cut->GetN(); ++i) 
			cut->SetPoint(i, newc->GetX()[i], newc->GetY()[i]);
		} 
		delete newc;

	} else {
		printf("SocketServer: Received unknown command \"%s\"\n", 
				request);
	}
	
	return 0;
}
