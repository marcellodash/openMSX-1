from urlparse import urljoin

class Package(object):
	'''Abstract base class for packages.
	'''
	niceName = None
	sourceName = None

	@classmethod
	def getMakeName(cls):
		return cls.sourceName.upper().replace('-', '_')

class DownloadablePackage(Package):
	'''Abstract base class for packages that can be downloaded.
	'''
	downloadURL = None
	version = None
	fileLength = None
	checksums = None

	@classmethod
	def getSourceDirName(cls):
		'''Returns the desired name of the top-level source directory.
		This might not match the actual name inside the downloaded archive,
		but we can perform a rename on extraction to fix that.
		'''
		return '%s-%s' % (cls.sourceName, cls.version)

	@classmethod
	def getTarballName(cls):
		return '%s-%s.tar.gz' % (cls.sourceName, cls.version)

	@classmethod
	def getURL(cls):
		return urljoin(cls.downloadURL + '/', cls.getTarballName())

class ALSA(DownloadablePackage):
	downloadURL = 'ftp://ftp.alsa-project.org/pub/lib/'
	niceName = 'ALSA'
	sourceName = 'alsa-lib'
	version = '1.1.7'
	fileLength = 1005257
	checksums = {
		'sha256':
			'9d6000b882a3b2df56300521225d69717be6741b71269e488bb20a20783bdc09',
		}

	@classmethod
	def getTarballName(cls):
		return '%s-%s.tar.bz2' % (cls.sourceName, cls.version)

	@classmethod
	def getMakeName(cls):
		return 'ALSA'

class FreeType(DownloadablePackage):
	downloadURL = 'http://downloads.sourceforge.net/freetype'
	niceName = 'FreeType'
	sourceName = 'freetype'
	version = '2.4.12'
	fileLength = 2117909
	checksums = {
		'sha256':
			'9755806ff72cba095aad47dce6f0ad66bd60fee2a90323707d2cac5c526066f0',
		}

class GLEW(DownloadablePackage):
	downloadURL = 'http://downloads.sourceforge.net/glew'
	niceName = 'GLEW'
	sourceName = 'glew'
	version = '1.9.0'
	fileLength = 544440
	checksums = {
		'sha256':
			'9b36530e414c95d6624be9d6815a5be1531d1986300ae5903f16977ab8aeb787',
		}

	@classmethod
	def getTarballName(cls):
		return '%s-%s.tgz' % (cls.sourceName, cls.version)

class LibPNG(DownloadablePackage):
	downloadURL = 'http://downloads.sourceforge.net/libpng'
	niceName = 'libpng'
	sourceName = 'libpng'
	version = '1.6.20'
	fileLength = 1417478
	checksums = {
		'sha256':
			'3d3bdc16f973a62fb1d26464fe2fe19f51dde9b883feff3e059d18ec1457b199',
		}

	@classmethod
	def getMakeName(cls):
		return 'PNG'

class OGG(DownloadablePackage):
	downloadURL = 'http://downloads.xiph.org/releases/ogg'
	niceName = 'libogg'
	sourceName = 'libogg'
	version = '1.3.3'
	fileLength = 579853
	checksums = {
		'sha256':
			'c2e8a485110b97550f453226ec644ebac6cb29d1caef2902c007edab4308d985',
		}

	@classmethod
	def getMakeName(cls):
		return 'OGG'

class OpenGL(Package):
	niceName = 'OpenGL'
	sourceName = 'gl'

class PkgConfig(DownloadablePackage):
	downloadURL = 'https://pkg-config.freedesktop.org/releases'
	niceName = 'pkg-config'
	sourceName = 'pkg-config'
	version = '0.29.2'
	fileLength = 2016830
	checksums = {
		'sha256':
			'6fc69c01688c9458a57eb9a1664c9aba372ccda420a02bf4429fe610e7e7d591',
		}

class SDL2(DownloadablePackage):
	downloadURL = 'http://www.libsdl.org/release'
	niceName = 'SDL2'
	sourceName = 'SDL2'
	version = '2.0.9'
	fileLength = 5246942
	checksums = {
		'sha256':
			'255186dc676ecd0c1dbf10ec8a2cc5d6869b5079d8a38194c2aecdff54b324b1',
		}

class SDL2_ttf(DownloadablePackage):
	downloadURL = 'http://www.libsdl.org/projects/SDL_ttf/release'
	niceName = 'SDL2_ttf'
	sourceName = 'SDL2_ttf'
	version = '2.0.14'
	fileLength = 4147462
	checksums = {
		'sha256':
			'34db5e20bcf64e7071fe9ae25acaa7d72bdc4f11ab3ce59acc768ab62fe39276',
		}

class TCL(DownloadablePackage):
	downloadURL = 'http://downloads.sourceforge.net/tcl'
	niceName = 'Tcl'
	sourceName = 'tcl'
	version = '8.5.18'
	fileLength = 4534628
	checksums = {
		'sha256':
			'032be57a607bdf252135b52fac9e3a7016e526242374ac7637b083ecc4c5d3c9',
		}

	@classmethod
	def getSourceDirName(cls):
		return '%s%s' % (cls.sourceName, cls.version)

	@classmethod
	def getTarballName(cls):
		return '%s%s-src.tar.gz' % (cls.sourceName, cls.version)

class Theora(DownloadablePackage):
	downloadURL = 'http://downloads.xiph.org/releases/theora'
	niceName = 'libtheora'
	sourceName = 'libtheora'
	version = '1.1.1'
	fileLength = 2111877
	checksums = {
		'sha256':
			'40952956c47811928d1e7922cda3bc1f427eb75680c3c37249c91e949054916b',
		}

	@classmethod
	def getMakeName(cls):
		return 'THEORA'

class Vorbis(DownloadablePackage):
	downloadURL = 'http://downloads.xiph.org/releases/vorbis'
	niceName = 'libvorbis'
	sourceName = 'libvorbis'
	version = '1.3.6'
	fileLength = 1634357
	checksums = {
		'sha256':
			'6ed40e0241089a42c48604dc00e362beee00036af2d8b3f46338031c9e0351cb',
		}

	@classmethod
	def getMakeName(cls):
		return 'VORBIS'

class ZLib(DownloadablePackage):
	downloadURL = 'http://downloads.sourceforge.net/libpng'
	niceName = 'zlib'
	sourceName = 'zlib'
	version = '1.2.8'
	fileLength = 571091
	checksums = {
		'sha256':
			'36658cb768a54c1d4dec43c3116c27ed893e88b02ecfcb44f2166f9c0b7f2a0d',
		}

# Build a dictionary of packages using introspection.
def _discoverPackages(localObjects):
	for obj in localObjects:
		if isinstance(obj, type) and issubclass(obj, Package):
			if not (obj is Package or obj is DownloadablePackage):
				yield obj.getMakeName(), obj
_packagesByName = dict(_discoverPackages(locals().itervalues()))

def getPackage(makeName):
	return _packagesByName[makeName]

def iterDownloadablePackages():
	for package in _packagesByName.itervalues():
		if issubclass(package, DownloadablePackage):
			yield package
