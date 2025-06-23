import groovy.transform.Field

@Field def AMPCXI_JENKINS_OWNER = 'Cloud'
@Field def AMPCXI_JENKINS_TAG = 'master'

node("ANSIBLE&&CONTROLLER") {
  library identifier: "ampcxi-jenkins-library@${AMPCXI_JENKINS_TAG}", retriever: modernSCM (
    scm: [
      $class: 'GitSCMSource',
      remote: "https://code.engine.sourcefire.com/${AMPCXI_JENKINS_OWNER}/ampcxi-jenkins",
      credentialsId: '6ea4923c-e6a6-4df4-82b4-9b1004c9c85b'
    ],
    libraryPath: 'lib'
  )
}

@Field def build_platforms

def linuxBuildPlatforms() {
  return [
    "alma8": ["tag": "el8"],
    "alma9": ["tag": "el9"],
    "ubuntu20": ["tag": "ubuntu20"],
    "ubuntu20arm": ["tag": "ubuntu20arm"],
    // "mac14arm": ["tag": "mac14arm"],
  ]
}

// CI Helpers
// ##### CI & Build Nodes Setup start.
def setup_ephemeral_nodes(platformList) {
  def parallelDeployNodes = [:]

  platformList.each { platform ->
    parallelDeployNodes[platform] = {
      def node_label = pipeline_utils.isBranch() ? pipeline_utils.getReleaseNodeLabel(platform) : pipeline_utils.getCINodeLabel(platform)
      def node = pipeline_utils.filter_nodes(node_label, false)
      if (node.size()) {
        env."${platform}" = node.first()
      } else {
        def os = platform.replaceAll("-aws","")
        if (os == "suse15") { // AWS/Provision-Manager uses 'opensuse15' to deploy 'suse15'
          os = "opensuse15"
        }

        env."${platform}" = aws_utils.deployCINodes("daily", os)[0]
      }
    }
  }
  parallel parallelDeployNodes
}
// ##### CI & Build Nodes setup end.

def isPullRequest() {
    return env.CHANGE_ID != null
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

def checkout_cmclient(owner, branch) {
  if (isPullRequest() && hasLabel(env.CLEAN_WS_LABEL)) {
    echo "${env.CLEAN_WS_LABEL} label found in pull request; cleaning workspace before checkout"
    deleteDir()
  }

  echo "Checking out https://code.engine.sourcefire.com/${owner}/cm-client:${branch}"

  checkout([
    $class: 'GitSCM',
    branches: [[name: "*/${branch}"]],
    userRemoteConfigs: [[ url: "https://code.engine.sourcefire.com/${owner}/cm-client", credentialsId: "6ea4923c-e6a6-4df4-82b4-9b1004c9c85b" ]],
    doGenerateSubmoduleConfigurations: false,
    extensions: [
      [ $class: 'SubmoduleOption',
        disableSubmodules: false,
        parentCredentials: true,
        recursiveSubmodules: true,
        reference: '',
        threads: 8, // default 1, max 8
        shallow: true,
        trackingSubmodules: false],
      [ $class: 'RelativeTargetDirectory', relativeTargetDir: 'cm-client']
    ]
  ])
}

def generate_checkout(platform, owner, tag) {
  return {
    node(env."${platform}") {
      def AMPCX_TAG = env.CHANGE_BRANCH == null ? env.BRANCH_NAME : env.CHANGE_BRANCH
      env."CUSTOM_WORKSPACE_${platform}" = "${WORKSPACE}/${AMPCX_TAG}"
      ws(evaluate("env.CUSTOM_WORKSPACE_${platform}")) {
        stage(platform) {
          checkout_cmclient(owner, tag)
        }
      }
    }
  }
}

def generate_build(platform) {
  return {
    node(env."${platform}") {
      ws(evaluate("env.CUSTOM_WORKSPACE_${platform}")) {
        stage(platform) {
          pipeline_utils.printNode()
          if (continueCI()) {
            withEnv(['PATH+=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/bin:/var/lib/jenkins/go/bin']) {
              withVault([configuration: vault_utils.staging_config(), vaultSecrets: vault_utils.get_secrets("artifactory")]) {
                dir("cm-client") {
                  if (platform.toLowerCase().contains("mac")) {
                    sh './build -c'
                    sh './build -d'
                  } else {
                    // Run with devtoolset on Linux, where applicable
                    sh """
                      ${pipeline_utils.prependDevtoolset(platform)} ./build -d
                    """
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

def generate_release_build(platform) {
  return {
    node(env."${platform}") {
      ws(evaluate("env.CUSTOM_WORKSPACE_${platform}")) {
        stage(platform) {
          pipeline_utils.printNode()
          if (continueCI()) {
            withEnv(['PATH+=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/bin:/var/lib/jenkins/go/bin']) {
              dir("cm-client") {
                env.CM_BUILD_VER = "1.0.0000"
                if (platform.toLowerCase().contains("mac")) {
                  sh './build -r'
                } else {
                  // Run with devtoolset on Linux, where applicable
                  sh """
                    ${pipeline_utils.prependDevtoolset(platform)} ./build -r
                  """
                }
              }
            }
          }
        }
      }
    }
  }
}

def generate_test_build(platform) {
  return {
    node(env."${platform}") {
      ws(evaluate("env.CUSTOM_WORKSPACE_${platform}")) {
        stage(platform) {
          pipeline_utils.printNode()
          if (continueCI()) {
            withEnv(['PATH+=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/bin:/var/lib/jenkins/go/bin']) {
              dir("cm-client") {
                if (platform.toLowerCase().contains("mac")) {
                  // Do this for each individual test suite, i.e. add PackageManager when available
                  dir("cm-client/debug/client/tests"){
                    sh 'ctest --output-on-failure'
                  }
                  dir("cm-client/debug/OSPackageManager/tests"){
                    sh 'ctest --output-on-failure'
                  }
                } else {
                  // Do this for each individual test suite, i.e. add PackageManager when available
                  dir("cm-client/debug/client/tests"){
                    sh 'ctest --output-on-failure'
                  }
                  // TODO OSPackageManager tests currently disabled on Linux
                }
              }
            }
          }
        }
      }
    }
  }
}

def generate_xcode_build(platform) {
  return {
    node(env."${platform}") {
      ws(evaluate("env.CUSTOM_WORKSPACE_${platform}")) {
        stage(platform) {
          pipeline_utils.printNode()
          if (continueCI()) {
            withEnv(['PATH+=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/bin:/var/lib/jenkins/go/bin']) {
              dir("cm-client") {
                if (platform.toLowerCase().contains("mac")) {
                  sh './build -c'
                  sh './build -x -d'
                  sh 'xcodebuild build -project xcode_debug/cm-client.xcodeproj -configuration "Debug" -scheme ALL_BUILD'
                } else {
                  echo "Xcode build not applicable on Linux"
                }
              }
            }
          }
        }
      }
    }
  }
}

def generate_xcode_release_build(platform) {
  return {
    node(env."${platform}") {
      ws(evaluate("env.CUSTOM_WORKSPACE_${platform}")) {
        stage(platform) {
          pipeline_utils.printNode()
          if (continueCI()) {
            withEnv(['PATH+=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/bin:/var/lib/jenkins/go/bin']) {
              dir("cm-client") {
                env.CM_BUILD_VER = "1.0.0000"
                if (platform.toLowerCase().contains("mac")) {
                  env.CM_BUILD_VER = "1.0.0000"
                  sh './build -x -r'
                  sh 'xcodebuild build -project xcode_release/cm-client.xcodeproj -configuration "Release" -scheme ALL_BUILD'
                } else {
                  echo "Xcode build not applicable on Linux"
                }
              }
            }
          }
        }
      }
    }
  }
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

def run_cmclient_ci() {
  stage('Setup Ephemeral Nodes') {
    build_platforms = linuxBuildPlatforms().keySet()
    println "Build Platforms: ${build_platforms.join(',')}"

    setup_ephemeral_nodes(build_platforms)
    println build_platforms.collectEntries { ["${it}" : env."${it}"] }
  }
  stage('Checkout') {
    def parallel_checkout = build_platforms.collectEntries {
      ["${it}" : generate_checkout(it, env.CHANGE_FORK, env.CHANGE_BRANCH)]
    }
    parallel parallel_checkout
  }
  stage("Build Debug") {
    parallel_build = build_platforms.collectEntries {
      ["${it}" : generate_build(it)]
    }
    parallel parallel_build
  }
  stage("Build Release") {
    if (continueCI()) {
      parallel_build = build_platforms.collectEntries {
        ["${it}" : generate_release_build(it)]
      }
      parallel parallel_build
    }
  }
  stage("Test") {
    if (continueCI()) {
      parallel_build = build_platforms.collectEntries {
        ["${it}" : generate_test_build(it)]
      }
      parallel parallel_build
    }
  }
  stage("Build Xcode Debug") {
    parallel_build = build_platforms.collectEntries {
      ["${it}" : generate_xcode_build(it)]
    }
    parallel parallel_build
  }
  stage("Build Xcode Release") {
    if (continueCI()) {
      parallel_build = build_platforms.collectEntries {
        ["${it}" : generate_xcode_release_build(it)]
      }
      parallel parallel_build
    }
  }
  // TODO Add test stage when available
}

properties([
  parameters([
    string(name: 'AMPCXI_JENKINS_OWNER', defaultValue: "Cloud"),
    string(name: 'AMPCXI_JENKINS_TAG', defaultValue: "master")
  ])
])

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
    stage('cmclient') {
      agent {
        node {
          label "ANSIBLE&&CONTROLLER"
        }
      }
      steps {
        script {
          run_cmclient_ci()
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
