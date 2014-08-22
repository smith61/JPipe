package jpipe;

import java.io.Closeable;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * @author Jacob
 *
 * This class allows access to the native Pipe interface
 *  provided by most underlying OSes. Pipes are able to
 *  be accessed by FileInputStream/FileOutputStream but
 *  you can not create a Pipes with the existing API.
 *  
 * This class allows you to create and use pipes with
 *  the same API. Pipes can be Read/Write only if the
 *  underlying OS supports Read/Write Pipes. 
 *  
 *  Currently only Win32 systems are supported.
 *  
 * 
 * Closing any stream associated with a pipe or closing
 * 	the pipe itself, closes all stream related to the pipe
 */
public class JPipe implements Closeable, AutoCloseable {

	public static final int READ              = 1 << 0;
	public static final int WRITE             = 1 << 1;
	public static final int CREATE            = 1 << 2;
	
	public static final int WRITE_CREATE      = JPipe.WRITE | JPipe.CREATE;
	public static final int READ_CREATE       = JPipe.READ | JPipe.CREATE;
	public static final int READ_WRITE        = JPipe.READ | JPipe.WRITE;
	public static final int READ_WRITE_CREATE = JPipe.READ_WRITE | JPipe.CREATE;
	
	private static native FileDescriptor open( String name, int mode ) throws IOException;
	
	public static JPipe openPipe( String name, int mode ) throws IOException {
		if( name == null || name.isEmpty() || ( name = name.trim() ).isEmpty() ) {
			throw new IOException( "Invalid name: " + name );
		}
		else if( mode > JPipe.READ_WRITE_CREATE ) {
			throw new IOException( "Invalid mode: " + mode );
		}
		else if( ( mode & ( JPipe.READ | JPipe.WRITE ) ) == 0 ) {
			throw new IOException( "Invalid mode: " + mode );
		}
		
		return new JPipe( name, mode, JPipe.open( name, mode ) );
	}
	
	private final String name;
	private final int mode;
	
	private final JPipeOutputStream out;
	private final JPipeInputStream in;
	
	private JPipe( String name, int mode, FileDescriptor fd ) {
		this.name = name;
		this.mode = mode;
		
		if( ( mode & JPipe.READ ) != 0 ) {
			this.in = new JPipeInputStream( fd );
		}
		else {
			this.in = null;
		}
		
		if( ( mode & JPipe.WRITE ) != 0 ) {
			this.out = new JPipeOutputStream( fd );
		}
		else {
			this.out = null;
		}
	}
	
	public FileInputStream inputStream() {
		return this.in;
	}
	
	public FileOutputStream outputStream() {
		return this.out;
	}
	
	public void close() throws IOException {
		// One of the streams needs to close the FD because
		//  we can't access it. Doesn't matter which stream
		if( out != null ) {
			out.closeImp();
		}
		else {
			in.closeImp();
		}
	}
	
	static {
		System.loadLibrary( "JPipe" );
	}
	
	private class JPipeInputStream extends FileInputStream {
		
		public JPipeInputStream( FileDescriptor fd ) {
			super( fd );
		}
		
		public void close() throws IOException {
			JPipe.this.close();
		}
		
		public void closeImp() throws IOException {
			super.close();
		}
	}
	
	private class JPipeOutputStream extends FileOutputStream {
		
		public JPipeOutputStream( FileDescriptor fd ) {
			super( fd );
		}
		
		public void close() throws IOException {
			JPipe.this.close();
		}
		
		public void closeImp() throws IOException {
			super.close();
		}
	}
}
