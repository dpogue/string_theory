#!/usr/bin/env groovy

pipeline {
  agent {
    label 'solaris'
  }

  environment {
    PATH = "/opt/local/bin:$PATH"
    CC = 'gcc'
    CXX = 'g++'
  }

  stages {
    stage('GitHub Checkout') {
      steps {
        checkout scm: [$class: 'GitSCM',
                  branches: [[name: '*/jenkins']],
                  doGenerateSubmoduleConfigurations: false,
                  extensions: [],
                  submoduleCfg: [],
                  userRemoteConfigs: [[url: 'https://github.com/dpogue/string_theory.git']]]
      }
    }

    stage('Build') {
      steps {
        sh "mkdir -p build"

        dir("build") {
          sh "cmake -DCMAKE_BUILD_TYPE=Debug -DST_BUILD_TESTS=ON -DCMAKE_INSTALL_PREFIX=${env.WORKSPACE}/artifacts .."
          sh "make"
        }
      }
    }

    stage('Test') {
      steps {
        dir("build") {
          sh "make test"
        }
      }
    }

    stage('Install') {
      steps {
        dir("build") {
          sh "make install"
        }
      }

      post {
        success {
          archiveArtifacts artifacts: 'artifacts/**/*', fingerprint: true
        }
      }
    }
  }

  post {
    always {
      emailext attachLog: true,
               body: '''${JELLY_SCRIPT, template="html"}''',
               recipientProviders: [developers(), requestor()],
               subject: "${env.JOB_NAME} Build #${env.BUILD_NUMBER}: ${currentBuild.currentResult}"
    }
  }
}
