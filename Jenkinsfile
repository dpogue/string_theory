#!/usr/bin/env groovy

pipeline {
    agent {
        label 'solaris'
    }

    stages {
        stage('Build and Test') {
            environment {
                CC = 'gcc'
                CXX = 'g++'
            }

            steps {
                checkout scm

                sh "mkdir -p build"
                dir build

                sh "cmake -DCMAKE_BUILD_TYPE=Debug -DST_BUILD_TESTS=ON .."
                sh "make"
                sh "make test"
            }
        }
    }
}
