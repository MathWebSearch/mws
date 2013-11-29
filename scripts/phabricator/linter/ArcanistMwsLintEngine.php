<?php
/*
 */
final class ArcanistMwsLintEngine extends ArcanistLintEngine {
  public function buildLinters() {
    $linters = array();
    $paths = $this->getPaths();

    foreach ($paths as $key => $path) {
      if (!$this->pathExists($path)) {
        unset($paths[$key]);
      }
      if (preg_match('@^(third_party|data)/@', $path)) {
        // Don't run lint engines on data and external code
        unset($paths[$key]);
      }
    }

    $text_paths = preg_grep('/\.(php|css|hpp|cpp|txt|y)$/', $paths);
    $linters[] = id(new ArcanistApacheLicenseLinter())->setPaths($text_paths);
    $linters[] = id(new ArcanistGeneratedLinter())->setPaths($text_paths);
    $linters[] = id(new ArcanistNoLintLinter())->setPaths($text_paths);
    $linters[] = id(new ArcanistTextLinter())->setPaths($text_paths);
    $linters[] = id(new ArcanistFilenameLinter())->setPaths($paths);

    $linters[] = id(new ArcanistXHPASTLinter())
        ->setPaths(preg_grep('/\.php$/', $paths));

    $linters[] = id(new ArcanistCpplintLinter())
        ->setPaths(preg_grep('/\.(cpp|c|hpp|h)$/', $paths));

    $linters[] = id(new ArcanistFlake8Linter())
         ->setPaths(preg_grep('/\.py$/', $paths));

    $linters[] = id(new ArcanistRubyLinter())
         ->setPaths(preg_grep('/\.rb$/', $paths));

    $linters[] = id(new ArcanistScalaSBTLinter())
         ->setPaths(preg_grep('/\.scala$/', $paths));

    $linters[] = id(new ArcanistJSHintLinter())
         ->setPaths(preg_grep('/\.js$/', $paths));

    return $linters;
  }
}
