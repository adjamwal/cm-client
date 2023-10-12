pipeline {
  options {
    // Disable concurrent builds of the same branch
    disableConcurrentBuilds()
    timestamps()
  }
  // Unassign default agent/slave/node. They will be specified separately in
  // eac
  agent none
  environment {
    ARCHIVE_NAME="${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_BUILD}"
    CM_BUILD_VER="${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_BUILD}"
  }

  //default parameters values forced is not adviced if being used for
  //multiple branches.

  stages {
    stage('Init') {
      steps {
        script {
          echo "Starting Build With Parameters:"
          echo "VERSION_MAJOR: $VERSION_MAJOR"
          echo "VERSION_MINOR: $VERSION_MINOR"
          echo "VERSION_PATCH: $VERSION_PATCH"
          echo "VERSION_BUILD: $VERSION_BUILD"
          echo "CM_CLIENT_OWNER: $CM_CLIENT_OWNER"
          echo "CM_CLIENT_TAG: $CM_CLIENT_TAG"
        }
      }
    }
    stage('Checkout/Build') {
      when {
        expression {
          currentBuild.result == null
        }
      }
      agent {
        node {
          label BUILD_SLAVE
        }
      }
      steps {
        script {
          currentBuild.description = "$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH.$VERSION_BUILD - $BUILD_NOTES"
        }
        cleanWs()
        checkout([$class: 'GitSCM',\
          branches: [[name: '*/$CM_CLIENT_TAG']],\
          doGenerateSubmoduleConfigurations: false,\
          extensions: [[$class: 'SubmoduleOption',\
            disableSubmodules: false,\
            parentCredentials: true,\
            recursiveSubmodules: true, reference: '',\
            trackingSubmodules: false],\
            [$class: 'RelativeTargetDirectory',\
              relativeTargetDir: 'cm-client']],\
          submoduleCfg: [],\
          userRemoteConfigs: scm.userRemoteConfigs])
        withCredentials([usernamePassword(credentialsId: 'mac-build-slave-10.12-xcode9-build',\
                         passwordVariable: 'BUILD_PASS', usernameVariable: 'BUILD_USER'),\
                         usernamePassword(credentialsId: 'notarization_ampcxi_credentials',\
                         passwordVariable: 'NOTARIZATION_PASS', usernameVariable: 'NOTARIZATION_USER') ]) {
          dir('cm-client') {
            withEnv(['PATH+=/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/Users/build/go/bin:/Users/build/bin']) {
                sh 'security -v unlock-keychain -p "$BUILD_PASS"'
                sh 'scripts/jenkins/create_bs_json.sh "$CM_BUSINESS_ID" "$CM_EVENT_URL" "$CM_IDENTIFY_URL" "$CM_INSTALLER_KEY" client/config/bs_release.json'
                sh 'scripts/build.sh -r'
            }
          }
        }

        sh 'cp cm-client/Staging/cisco-secure-client-macos-cloudmanagement-*-symbols.tgz .'
        sh 'cp cm-client/Staging/cisco-secure-client-macos-cloudmanagement-*.pkg .'
        sh 'cp cm-client/Staging/cisco-secure-client-macos-cloudmanagement-*.dmg .'
        sh 'cp cm-client/Staging/GIT_FP.txt .'
        
        withEnv(['PATH+=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/bin:/var/lib/jenkins/go/bin']) {
          withCredentials([string(credentialsId: 'ARTIFACTORY_TOKEN', variable: 'ARTIFACTORY_TOKEN')]) {
            dir('cm-client/Staging') {
              sh '../scripts/jenkins/artifactory_upload.sh "cisco-secure-client-macos-cloudmanagement-${CM_BUILD_VER}.pkg"'
              sh '../scripts/jenkins/artifactory_upload.sh "cisco-secure-client-macos-cloudmanagement-${CM_BUILD_VER}-symbols.tgz"'
              sh '../scripts/jenkins/artifactory_upload.sh "cisco-secure-client-macos-cloudmanagement-testpackage1-${CM_BUILD_VER}.pkg"'
            }
          }
        }

        stash includes: '\
          GIT_FP.txt, \
          cisco-secure-client-macos-cloudmanagement-*', \
          name: 'secure-client-cloud-management-build'
      }
    }
    stage('Archive') {
      when {
        expression {
          currentBuild.result == null
        }
      }
      agent {
        node {
          label 'linux-centos7-build-slave'
        }
      }
      steps {
        cleanWs()
        unstash 'secure-client-cloud-management-build'
        script {
          sh 'mkdir ./${ARCHIVE_NAME}'
          sh 'cp cisco-secure-client-macos-cloudmanagement-* ./${ARCHIVE_NAME}'
          sh 'cp GIT_FP.txt ./${ARCHIVE_NAME}'
        }
        dir(env.ARCHIVE_NAME) {
          archiveArtifacts artifacts: '**'
        }
      }
    }
  }
  post {
    failure {
      script {
        if(env.EMAIL == "YES") {
          subject = "Build Failure: Cisco Secure Cloud Management release $VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH.$VERSION_BUILD"
          subject = subject + " Build Failed."

          emailext to: '$NOTIFICATION_RECIPIENTS',\
          subject: subject,\
          body: "Hello,<br/><br/>Secure Client Cloud Management release $VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH.$VERSION_BUILD Build Failed.<br/><br/>Jenkins Job URL: ${env.BUILD_URL}<br/><br/>Blue Ocean URL: ${env.RUN_DISPLAY_URL}",
          mimeType: 'text/html'
        }
      }
    }
  }
}
