# $Id$

from components import requiredLibrariesFor
from configurations import getConfiguration
from download import downloadURL
from extract import TopLevelDirRenamer, extract
from libraries import librariesByName
from packages import getPackage
from patch import Diff, patch

from os.path import isdir, isfile, join as joinpath
from shutil import rmtree
import sys

# TODO: Make DirectX headers for MinGW a package and make the DirectX sound
#       driver a component.

def downloadPackage(package, tarballsDir):
	if isfile(joinpath(tarballsDir, package.getTarballName())):
		print '%s version %s - already downloaded' % (
			package.niceName, package.version
			)
	else:
		downloadURL(package.getURL(), tarballsDir)

def extractPackage(package, tarballsDir, sourcesDir, patchesDir):
	sourceDirName = package.getSourceDirName()
	packageSrcDir = joinpath(sourcesDir, sourceDirName)
	if isdir(packageSrcDir):
		rmtree(packageSrcDir)
	extract(
		joinpath(tarballsDir, package.getTarballName()),
		sourcesDir,
		TopLevelDirRenamer(sourceDirName)
		)
	diffPath = joinpath(patchesDir, sourceDirName + '.diff')
	if isfile(diffPath):
		for diff in Diff.load(diffPath):
			patch(diff, sourcesDir)
			print 'Patched:', diff.getPath()

def main(platform, tarballsDir, sourcesDir, patchesDir):
	if not isdir(tarballsDir):
		print >> sys.stderr, \
			'Output directory "%s" does not exist' % tarballsDir
		sys.exit(2)
	if not isdir(sourcesDir):
		print >> sys.stderr, \
			'Output directory "%s" does not exist' % sourcesDir
		sys.exit(2)

	configuration = getConfiguration('3RD_STA')

	# Compute the set of all directly and indirectly required libraries.
	transLibs = set()
	for makeName in requiredLibrariesFor(configuration.iterDesiredComponents()):
		library = librariesByName[makeName]
		transLibs.add(makeName)
		for depMakeName in library.dependsOn:
			transLibs.add(depMakeName)
	# Filter out system libraries.
	thirdPartyLibs = set(
		makeName
		for makeName in transLibs
		if not librariesByName[makeName].isSystemLibrary(platform)
		)

	for makeName in sorted(thirdPartyLibs):
		package = getPackage(makeName)
		downloadPackage(package, tarballsDir)
		extractPackage(package, tarballsDir, sourcesDir, patchesDir)

if __name__ == '__main__':
	if len(sys.argv) == 5:
		main(*sys.argv[1 : ])
	else:
		print >> sys.stderr, (
			'Usage: python 3rdparty_download.py '
			'TARGET_OS TARBALLS_DIR SOURCES_DIR PATCHES_DIR'
			)
		sys.exit(2)
