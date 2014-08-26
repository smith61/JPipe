import os
import sys

supported_platforms = [ 'win32', 'linux2' ]

if not sys.platform in supported_platforms:
    print 'Error: \'', sys.platform, '\' is not a supported platform.'
    sys.exit( -1 )

env = Environment( ENV = os.environ )
env[ 'JAVA_HOME' ] = os.environ[ 'JAVA_HOME' ]
env.Append( CPPPATH = [ '$JAVA_HOME/include' ])

if sys.platform == 'win32':
    env.Append( CPPPATH = [ '$JAVA_HOME/include/win32' ] )
elif sys.platform == 'linux2':
    env.Append( CPPPATH = [ '$JAVA_HOME/include/linux' ] )
    

env[ 'INSTALL_DIR' ] = Dir( './out' )

SConscript( "src/SConscript", variant_dir = "build", exports = 'env', duplicate = 0 )