module.exports = {
    devServer: {
        proxy: {
            '/api': {
                target: 'https://maker.ifttt.com/trigger/',
                ws: true,
                changeOrigin: true,
                pathRewrite: {
                    '^/api': ''
                },
            },
        }
    }
};