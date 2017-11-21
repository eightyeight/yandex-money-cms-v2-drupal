<?php

// autoload_static.php @generated by Composer

namespace Composer\Autoload;

class ComposerStaticInite157eff731a1e60581361fc324813b36
{
    public static $prefixLengthsPsr4 = array (
        'Y' => 
        array (
            'YaMoney\\CodeGeneratorBundle\\' => 28,
            'YaMoney\\' => 8,
        ),
        'P' => 
        array (
            'Psr\\Log\\' => 8,
        ),
    );

    public static $prefixDirsPsr4 = array (
        'YaMoney\\CodeGeneratorBundle\\' => 
        array (
            0 => __DIR__ . '/..' . '/yandex-money/yandex-money-sdk-php/code-generator/CodeGeneratorBundle',
        ),
        'YaMoney\\' => 
        array (
            0 => __DIR__ . '/..' . '/yandex-money/yandex-money-sdk-php/lib',
        ),
        'Psr\\Log\\' => 
        array (
            0 => __DIR__ . '/..' . '/psr/log/Psr/Log',
        ),
    );

    public static function getInitializer(ClassLoader $loader)
    {
        return \Closure::bind(function () use ($loader) {
            $loader->prefixLengthsPsr4 = ComposerStaticInite157eff731a1e60581361fc324813b36::$prefixLengthsPsr4;
            $loader->prefixDirsPsr4 = ComposerStaticInite157eff731a1e60581361fc324813b36::$prefixDirsPsr4;

        }, null, ClassLoader::class);
    }
}