import Foundation

/// Saves/loads a `Project` as JSON, plus a small "recent projects" list in
/// UserDefaults. Sample files are referenced by URL bookmark so a project
/// still finds its samples after files are renamed or moved on the same
/// volume; the raw `sampleURL` is kept too as a fallback for when the
/// bookmark can't be resolved (moved to a different machine, etc).
final class ProjectStore {
    static let fileExtension = "octaclone"
    private static let recentsKey = "OctaClone.RecentProjects"

    func save(_ project: Project, to url: URL) throws {
        for track in project.tracks {
            if let sampleURL = track.sampleURL {
                track.sampleBookmark = try? sampleURL.bookmarkData()
            }
        }
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        let data = try encoder.encode(project)
        try data.write(to: url, options: .atomic)
        addToRecents(url)
    }

    func load(from url: URL) throws -> Project {
        let data = try Data(contentsOf: url)
        let project = try JSONDecoder().decode(Project.self, from: data)
        project.tracks.forEach(resolveSampleURL)
        addToRecents(url)
        return project
    }

    private func resolveSampleURL(for track: Track) {
        guard let bookmark = track.sampleBookmark else { return }
        var isStale = false
        guard let resolved = try? URL(resolvingBookmarkData: bookmark, options: [], relativeTo: nil, bookmarkDataIsStale: &isStale) else {
            return // fall back to whatever sampleURL was decoded from JSON, if any
        }
        track.sampleURL = resolved
    }

    // MARK: - Recent projects

    func recentProjectURLs() -> [URL] {
        let raw = UserDefaults.standard.stringArray(forKey: Self.recentsKey) ?? []
        return raw.compactMap(URL.init(string:)).filter { FileManager.default.fileExists(atPath: $0.path) }
    }

    private func addToRecents(_ url: URL) {
        var urls = recentProjectURLs().filter { $0 != url }
        urls.insert(url, at: 0)
        urls = Array(urls.prefix(10))
        UserDefaults.standard.set(urls.map(\.absoluteString), forKey: Self.recentsKey)
    }
}
