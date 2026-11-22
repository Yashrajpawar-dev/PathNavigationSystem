/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        navy: {
          900: '#0F172A',
          800: '#1E293B',
          700: '#1E3A8A', // Primary Navy from spec
          600: '#1D4ED8',
        },
        primary: '#1E3A8A',
        accent: '#3B82F6',
        success: '#16A34A',
        warning: '#F59E0B',
        danger: '#DC2626',
        surface: '#FFFFFF',
        bg: '#F8FAFC',
        border: '#E5E7EB',
      },
      fontFamily: {
        sans: ['Inter', 'system-ui', 'sans-serif'],
      }
    },
  },
  plugins: [],
}
