def isPullRequest() {
    return env.CHANGE_ID != null
}

def cmclientCheckout(recursiveSubmodules) {
  checkout([$class: 'GitSCM',
  branches: [[name: isPullRequest() ? 'refs/remotes/origin/PR-${CHANGE_ID}' : '${BRANCH_NAME}']],
  doGenerateSubmoduleConfigurations: false,
  extensions: [[$class: 'SubmoduleOption',
  disableSubmodules: false,
  parentCredentials: true,
  recursiveSubmodules: recursiveSubmodules,
  reference: '',
  trackingSubmodules: false],
  [$class: 'RelativeTargetDirectory', relativeTargetDir: 'cm-client']],
  submoduleCfg: [],
  userRemoteConfigs: scm.userRemoteConfigs])
}

//Perform a checkout of the cm-client repo into a subfolder and update all submodules
def repoCheckout(platform) {
  if (!isPullRequest()){
    echo "Branch Build; cleaning workspace before checkout"
    deleteDir()
  }

  if (isPullRequest() && hasLabel(env.CLEAN_WS_LABEL)) {
    echo "${env.CLEAN_WS_LABEL} label found in pull request; cleaning workspace before checkout"
    deleteDir()
  }

  cmclientCheckout(true)
}

def continueCI() {
  if (currentBuild.result == 'FAILURE') {
    echo "A failure occurred in another stage. Skipping..."
    return false
  }
  else if (currentBuild.result == 'SUCCESS') {
    echo "CI not required. Skipping..."
    return false
  }
  return true
}

def hasLabel(target_label) {
  def retval = false
  for (label in pullRequest.labels) {
    if (label.equals(target_label)) {
      retval = true
      break;
    }
  }
  return retval;
}

// TODO: Add Xcode build stage when Xcode build is more reliable
def run_mac_ci() {
  stage('Mac Checkout') {
    if (continueCI()) {
      repoCheckout("mac")
    }
  }
  stage("Build Debug") {
    if (continueCI()) {
      withEnv(['PATH+=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/bin:/var/lib/jenkins/go/bin']) {
        dir("cm-client"){
          sh './build'
        }
      }
    }
  }
  stage("Build Release") {
    if (continueCI()) {
      withEnv(['PATH+=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/bin:/var/lib/jenkins/go/bin']) {
        dir("cm-client"){
          sh './build -r'
        }
      }
    }
  }
  stage("Test") {
    if (continueCI()) {
      dir("cm-client/debug"){
        // Is this ready yet?
        //sh 'make test'
        echo "TODO: Add Tests"
      }
    }
  }
}

pipeline {
  options {
    // Disable concurrent builds of the same branch
    disableConcurrentBuilds()
    // Discard old builds
    buildDiscarder(logRotator(numToKeepStr:'15'))
    timeout(time: 2, unit: 'HOURS')
    timestamps()
  }
  // Unassign default agent/slave/node. They will be specified separately in
  // each stage of the pipeline.
  agent none
  environment {
    NOTIFICATION_RECIPIENTS = 'ampcxi-dev@cisco.com'
    // Number of lines to include in build failure notification emails
    BUILD_LOG_LINES           = '500'
    DISABLE_CI_LABEL          = 'noci'
    CLEAN_WS_LABEL            = 'cws'
    FORCE_BUILD_PREREQ_LABEL  = 'buildprereq'
    MASTER_BRANCH_NAME        = 'main'
    RELEASE_BRANCH_PREFIX     = 'rb-'
    WANT_CI_BRANCH_PREFIX     = 'ci-'
    COV_PLATFORM_PUBLISH      = 'yes'
  }
  stages {
    // If this build was triggered by a pull request: 1) manually determine
    // the change author's email (as CHANGE_AUTHOR_EMAIL is not always set in
    // the environment) and 2) look for DISABLE_CI_LABEL in the pull request
    // to see if the author requested CI workflow and email notifications to be
    // disabled
    stage('Init') {
      steps {
        script {
          if (isPullRequest()) {
            if (env.CHANGE_AUTHOR_DISPLAY_NAME == null) {
              env.CHANGE_AUTHOR_DISPLAY_NAME = env.CHANGE_AUTHOR
            }
            echo "Pull request initiated by ${CHANGE_AUTHOR_DISPLAY_NAME} (${CHANGE_AUTHOR})"
            echo "Checking for ${DISABLE_CI_LABEL} label in pull request ${CHANGE_ID} (${CHANGE_TITLE})"
            if (hasLabel(env.DISABLE_CI_LABEL)) {
              echo "${DISABLE_CI_LABEL} label found in pull request; set build result to success and finish early"
              currentBuild.result = 'SUCCESS'
              env.CHANGE_AUTHOR_EMAIL = ''
              return;
            }
            echo "${DISABLE_CI_LABEL} label not found in pull request; proceed with checkout"
            env.CHANGE_AUTHOR_EMAIL = env.CHANGE_AUTHOR + '@cisco.com'
            echo "Email notification will be sent to ${CHANGE_AUTHOR_EMAIL}"
        } else if (env.BRANCH_NAME != null) {
            if ((!env.BRANCH_NAME.equals(env.MASTER_BRANCH_NAME)) &&
              (!env.BRANCH_NAME.startsWith(env.RELEASE_BRANCH_PREFIX)) &&
              (!env.BRANCH_NAME.startsWith(env.WANT_CI_BRANCH_PREFIX))) {
              echo "${BRANCH_NAME} is neither main, a release branch, or a branch that explicitly requests CI and so will not be built; set build result to success and finish early"
              currentBuild.result = 'SUCCESS'
              env.CHANGE_AUTHOR_EMAIL = ''
              return;
            }
          }
        }
      }
    }
    stage('CI') {
      parallel {
        stage('macOS') {
          agent {
            label 'mac-build-orbital'
          }
          steps {
            script {
              run_mac_ci()
            }
          }
        }
      }
    }
    
  }
  post {
    success {
      script {
        if (!isPullRequest()) {
          // Only send notifications for branch builds when they recover
          if (currentBuild.previousBuild != null && currentBuild.previousBuild.result != 'SUCCESS') {
            emailext to: "${env.NOTIFICATION_RECIPIENTS}",\
            subject: "[JENKINS][cm-client] Branch ${env.JOB_BASE_NAME} (Fixed) #${env.BUILD_NUMBER}",\
            body: "Hello all,<br/><br/>Branch '${env.JOB_BASE_NAME}' is back to normal.<br/><br/>Blue Ocean URL: ${env.RUN_DISPLAY_URL}<br/>Job URL: ${env.BUILD_URL}",
            mimeType: 'text/html'
          }
        }
        else if (env.CHANGE_AUTHOR_EMAIL != null && !env.CHANGE_AUTHOR_EMAIL.isEmpty()) {
          // Send notification to pull request author
          emailext to: "${env.CHANGE_AUTHOR_EMAIL}",\
          subject: "[JENKINS][cm-client] Pull Request ${env.CHANGE_ID} (Successful) #${env.BUILD_NUMBER}",\
          body: "Hello ${env.CHANGE_AUTHOR_DISPLAY_NAME},<br/><br/>Pull request ${env.CHANGE_ID} '${env.CHANGE_TITLE}' successful.<br/><br/>Blue Ocean URL: ${env.RUN_DISPLAY_URL}<br/>Job URL: ${env.BUILD_URL}<br/>GitHub URL: ${env.CHANGE_URL}",
          mimeType: 'text/html'
        }
      }
    }
    failure {
      script {
        // Check if this was the first failure
        def firstFailure = currentBuild.previousBuild == null ||
        currentBuild.previousBuild.result != 'FAILURE'
        def statusText = firstFailure ? 'Failing' : 'Still Failing'
        if (!isPullRequest()) {
          emailext to: "${env.NOTIFICATION_RECIPIENTS}",\
          subject: "[JENKINS][cm-client] Branch ${env.JOB_BASE_NAME} (${statusText}) #${env.BUILD_NUMBER}",\
          body: "Hello all,<br/><br/>Branch ${env.JOB_BASE_NAME} failed to complete.<br/><br/>Blue Ocean URL: ${env.RUN_DISPLAY_URL}<br/>Job URL: ${env.BUILD_URL}</pre>",
          mimeType: 'text/html'
        }
        else if (env.CHANGE_AUTHOR_EMAIL != null && !env.CHANGE_AUTHOR_EMAIL.isEmpty()) {
          // Send notification to pull request author
          emailext to: "${env.CHANGE_AUTHOR_EMAIL}",\
          subject: "[JENKINS][cm-client] Pull Request ${env.CHANGE_ID} (${statusText}) #${env.BUILD_NUMBER}",\
          body: "Hello ${env.CHANGE_AUTHOR_DISPLAY_NAME},<br/><br/>Pull request ${env.CHANGE_ID} '${env.CHANGE_TITLE}' failed to build.<br/><br/>Blue Ocean URL: ${env.RUN_DISPLAY_URL}<br/>Job URL: ${env.BUILD_URL}<br/>GitHub URL: ${env.CHANGE_URL}</pre>",
          mimeType: 'text/html'
        }
      }
    }
  }
}
