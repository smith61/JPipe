/*
 * JPipe.cpp
 *
 *  Created on: Aug 21, 2014
 *      Author: Jacob
 */



#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <iostream>


#include "jni.h"

#define READ   ( 1 << 0 )
#define WRITE  ( 1 << 1 )
#define CREATE ( 1 << 2 )

#define OPT_SET( mode, opt ) ( ( ( mode ) & ( opt ) ) == opt )

static jclass    FILEDESCRIPTOR_CLASS  = NULL;
static jmethodID FILEDESCRIPTOR_CON    = 0;
static jfieldID  FILEDESCRIPTOR_HANDLE = 0;

static jclass IOEXCEPTION_CLASS = NULL;

static bool opt_set( jint mode, jint opt ) {
	return ( ( mode ) & opt ) == opt;
}

jobject JNICALL JPipe_open( JNIEnv *env, jclass, jstring jName, jint jMode ) {
	using namespace std;

	string errString = "";

	const jsize nameLength = env->GetStringUTFLength( jName );

	string path = "\\\\.\\pipe\\";
	const string::size_type size = path.size();
	path.resize( path.size() + nameLength );

	char *end = &path[ size ];

	env->GetStringUTFRegion( jName, 0, nameLength, end );

	DWORD mode = 0;
	if( opt_set( jMode, READ ) ) {
		mode |= GENERIC_READ;
	}
	if( opt_set( jMode, WRITE ) ) {
		mode |= GENERIC_WRITE;
	}

	HANDLE handle = CreateFile( path.c_str(), mode, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if( handle == INVALID_HANDLE_VALUE ) {
		if( GetLastError() == ERROR_FILE_NOT_FOUND && opt_set( jMode, CREATE ) ) {

			DWORD createMode = 0;
			if( opt_set( jMode, READ ) ) {
				createMode |= PIPE_ACCESS_INBOUND;
			}
			if( opt_set( jMode, WRITE ) ) {
				createMode |= PIPE_ACCESS_OUTBOUND;
			}
			handle = CreateNamedPipe( path.c_str(), createMode, PIPE_TYPE_BYTE, 1, 4096, 4096, 0, NULL );

			/* There exists a race condition where attempting to open
			 * both ends of the pipe at the same time causes them to attempt
			 * to each open the pipe. Only allowing one to actually create the
			 * pipe and the other to error out. Instead we attempt to reconnect
			 * to the pipe if we were unable to create it do to ERROR_PIPE_BUSY
			 */
			if( handle == INVALID_HANDLE_VALUE ) {

				if( GetLastError() == ERROR_PIPE_BUSY ) {
				    handle = CreateFile( path.c_str(), mode, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
				}
			}
			else {
				if( !ConnectNamedPipe( handle, NULL ) ) {
					CloseHandle( handle );
					handle = INVALID_HANDLE_VALUE;
				}
			}
		}
	}

	if( handle == INVALID_HANDLE_VALUE ) {
		char *errBuf;
		DWORD size = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
					   NULL,
					   GetLastError(),
					   MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
					   ( LPSTR ) &errBuf,
					   sizeof( errBuf ) - 1,
					   NULL );
		// Windows likes to tack on \r\n to the end of the message.
		//  Makes for a weird message.
		errBuf[ size - 2 ] = '\0';

		env->ThrowNew( IOEXCEPTION_CLASS, errBuf );
		LocalFree( errBuf );

		return NULL;
	}
	else {
		jobject fd = env->NewObject( FILEDESCRIPTOR_CLASS, FILEDESCRIPTOR_CON );
		env->SetLongField( fd, FILEDESCRIPTOR_HANDLE, ( long ) handle );

		return fd;
	}
}

JNINativeMethod nativeMethods[] = {
		{ "open", "(Ljava/lang/String;I)Ljava/io/FileDescriptor;", ( void * ) &JPipe_open }
};

JNIEXPORT jint JNICALL JNI_OnLoad( JavaVM *vm, void *resv ) {
	using namespace std;

	JNIEnv *env;

	int result = vm->GetEnv( ( void ** ) &env, JNI_VERSION_1_2 );
	if( result == JNI_OK ) {
		jclass clazz = env->FindClass( "jpipe/JPipe" );
		env->RegisterNatives( clazz, nativeMethods, sizeof( nativeMethods ) / sizeof( nativeMethods[ 0 ] ) );

		FILEDESCRIPTOR_CLASS  = ( jclass ) env->NewGlobalRef( env->FindClass( "java/io/FileDescriptor" ) );
		FILEDESCRIPTOR_CON    = env->GetMethodID( FILEDESCRIPTOR_CLASS, "<init>", "()V" );
		FILEDESCRIPTOR_HANDLE = env->GetFieldID( FILEDESCRIPTOR_CLASS, "handle", "J" );

		IOEXCEPTION_CLASS = ( jclass ) env->NewGlobalRef( env->FindClass( "java/io/IOException" ) );
	}
	else {
		vm->DestroyJavaVM();
	}

	return JNI_VERSION_1_2;
}
