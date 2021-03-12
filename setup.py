from setuptools import find_packages, setup

setup(
    name='edgerm',
    packages=find_packages(include=['scheduler*']),
    version='0.1.0',
    description='Edgerm scheduler library'
)
